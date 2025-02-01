# Vectorization 1

For a first vectorization example this took a bit longer to understand given the complexity of the existing code. The title of the lab gives it away, but might as well verify where the performance bottleneck is. Starting with high level metrics "backend_bound_cpu" seems promising.

```
$ perf stat -M PipelineL2 ./build/lab --benchmark_filter=baseline
2024-11-24T17:56:24-08:00
Running ./build/lab
Run on (16 X 5573 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 32768 KiB (x1)
Load Average: 0.22, 0.30, 0.20
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
baseline       810967 ns       810944 ns          863

 Performance counter stats for './build/lab --benchmark_filter=baseline':

         3,492,184      ex_ret_brn_misp                  #      0.4 %  bad_speculation_mispredicts
                                                  #      0.0 %  bad_speculation_pipeline_restarts  (24.99%)
    15,093,559,812      de_src_op_disp.all                                                      (24.99%)
             1,124      resyncs_or_nc_redirects                                                 (24.99%)
     4,366,254,752      ls_not_halted_cyc                                                       (24.99%)
    15,001,234,541      ex_ret_ops                                                              (24.99%)
         7,843,982      ex_no_retire.load_not_complete   #     39.0 %  backend_bound_cpu      
                                                  #      0.9 %  backend_bound_memory     (25.02%)
    10,474,861,146      de_no_dispatch_per_slot.backend_stalls                                        (25.02%)
       344,907,360      ex_no_retire.not_complete                                               (25.02%)
     4,370,599,879      ls_not_halted_cyc                                                       (25.02%)
         1,866,393      ex_ret_ucode_ops                 #     57.3 %  retiring_fastpath      
                                                  #      0.0 %  retiring_microcode       (25.02%)
     4,372,398,990      ls_not_halted_cyc                                                       (25.02%)
    15,041,870,347      ex_ret_ops                                                              (25.02%)
       607,301,545      de_no_dispatch_per_slot.no_ops_from_frontend #      1.6 %  frontend_bound_bandwidth  (24.96%)
        30,192,870      cpu/de_no_dispatch_per_slot.no_ops_from_frontend,cmask=0x6/ #      0.7 %  frontend_bound_latency   (24.96%)
     4,373,285,636      ls_not_halted_cyc                                                       (24.96%)

       0.791569296 seconds time elapsed

       0.791549000 seconds user
       0.000000000 seconds sys
```

Looking at the actual hot path of the code, as expected, it is clearly scalar code.

```
$ perf record ./build/lab --benchmark_filter=baseline
$ perf annotate -Mintel
  3.59 │260:   cmp        BYTE PTR [r8],r10b
  3.50 │       movzx      esi,WORD PTR [r15]
  3.37 │       mov        ecx,r11d
  3.11 │       cmovne     ecx,ebx
  2.92 │       add        r9d,ecx
  3.14 │       cmp        di,si
  3.69 │       mov        ecx,esi
  2.89 │       cmovge     ecx,edi
  3.27 │       cmp        r9w,cx
  3.46 │       cmovge     ecx,r9d
  3.11 │       movzx      r9d,WORD PTR [rdx]
  2.80 │       dec        edi
  3.62 │       mov        WORD PTR [rdx],cx
  3.14 │       sub        ecx,0xb
  4.42 │       cmp        di,cx
  5.43 │       cmovl      edi,ecx
  5.65 │       dec        esi
  5.74 │       cmp        si,cx
  7.05 │       cmovl      esi,ecx
  4.50 │       add        r15,0x2
  4.47 │       inc        r8
  5.49 │       add        rdx,0x2
  5.06 │       mov        WORD PTR [r15-0x2],si
  5.68 │       cmp        r12,r15
       │     ↑ jne        260
  0.32 │       mov        rsi,QWORD PTR [rsp+0x38]
  0.10 │       inc        rsi
  0.13 │       cmp        rsi,0xc8
```

Unfortunately I coded the solution a while ago at this point, but we can still compare the baseline versus both my coded solution and the "recommended" solution from the YouTube series.

```
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
baseline       809060 ns       808808 ns          865
solution       247518 ns       247508 ns         2829
video          261913 ns       261903 ns         2673
```

Both solutions rely on first transposing the data for better access patterns. There is of course a performance penalty for this processing, but the ability to process multiple comparisons at once is worth it. There's also likely room for improvement on the transposition logic, particularly doing the operation in-place could be explored. For now, however, just this still results in a noticable improvement.

The program no longer appears to be largely bound by backend_bound_cpu, but rather backend_bound_memory.

```
$ perf stat -M PipelineL2 ./build/lab --benchmark_filter=solution
2025-01-31T18:06:18-08:00
Running ./build/lab
Run on (16 X 5573 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1024 KiB (x8)
  L3 Unified 32768 KiB (x1)
Load Average: 0.04, 0.12, 0.20
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
solution       249424 ns       249383 ns         2815

 Performance counter stats for './build/lab --benchmark_filter=solution':

         1,295,009      ex_ret_brn_misp                  #      0.0 %  bad_speculation_mispredicts
                                                  #      0.0 %  bad_speculation_pipeline_restarts  (25.01%)
     7,932,465,405      de_src_op_disp.all                                                      (25.01%)
             1,011      resyncs_or_nc_redirects                                                 (25.01%)
     5,409,169,372      ls_not_halted_cyc                                                       (25.01%)
     7,919,661,057      ex_ret_ops                                                              (25.01%)
     3,657,100,890      ex_no_retire.load_not_complete   #      8.5 %  backend_bound_cpu      
                                                  #     65.8 %  backend_bound_memory     (25.00%)
    24,113,912,179      de_no_dispatch_per_slot.backend_stalls                                        (25.00%)
     4,129,309,128      ex_no_retire.not_complete                                               (25.00%)
     5,412,172,995      ls_not_halted_cyc                                                       (25.00%)
         4,242,200      ex_ret_ucode_ops                 #     24.3 %  retiring_fastpath      
                                                  #      0.0 %  retiring_microcode       (25.00%)
     5,414,195,115      ls_not_halted_cyc                                                       (25.00%)
     7,898,210,097      ex_ret_ops                                                              (25.00%)
       381,331,845      de_no_dispatch_per_slot.no_ops_from_frontend #      0.9 %  frontend_bound_bandwidth  (25.00%)
        17,090,918      cpu/de_no_dispatch_per_slot.no_ops_from_frontend,cmask=0x6/ #      0.3 %  frontend_bound_latency   (25.00%)
     5,415,506,619      ls_not_halted_cyc                                                       (25.00%)

       0.980376250 seconds time elapsed

       0.979215000 seconds user
       0.001000000 seconds sys
```

Further, as expected, much of the hotpath now uses vectorized code:

```
$ perf record ./build/lab --benchmark_filter=solution
$ perf annotate -Mintel
  3.58 │        vpmaxsw      xmm5,xmm5,xmm0
  4.05 │        vpmaxsw      xmm2,xmm2,xmm0
  7.87 │        vpmaxsw      xmm4,xmm4,xmm1
  7.55 │        vpmaxsw      xmm3,xmm3,xmm1
  8.15 │        vmovdqa      XMMWORD PTR [rdx-0x10],xmm2
  7.84 │        vmovdqa      XMMWORD PTR [rdx-0x20],xmm3
  4.39 │        vmovdqa      XMMWORD PTR [rsp+0x80],xmm4
  4.59 │        vmovdqa      XMMWORD PTR [rsp+0x90],xmm
  1.44 │        cmp          rcx,rsi
       │      ↑ jne          15f0
```
