# Local Results (Single Node)

LSCPU:

```
Architecture:            x86_64
  CPU op-mode(s):        32-bit, 64-bit
  Address sizes:         46 bits physical, 48 bits virtual
  Byte Order:            Little Endian
CPU(s):                  24
  On-line CPU(s) list:   0-23
Vendor ID:               GenuineIntel
  Model name:            12th Gen Intel(R) Core(TM) i9-12900K
    CPU family:          6
    Model:               151
    Thread(s) per core:  2
    Core(s) per socket:  16
    Socket(s):           1
    Stepping:            2
    CPU(s) scaling MHz:  34%
    CPU max MHz:         5200.0000
    CPU min MHz:         800.0000
    BogoMIPS:            6374.40
    Flags:               fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflu
                         sh dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm const
                         ant_tsc art arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc cpuid a
                         perfmperf tsc_known_freq pni pclmulqdq dtes64 monitor ds_cpl vmx smx est tm2 
                         ssse3 sdbg fma cx16 xtpr pdcm sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_
                         timer aes xsave avx f16c rdrand lahf_lm abm 3dnowprefetch cpuid_fault epb ssb
                         d ibrs ibpb stibp ibrs_enhanced tpr_shadow flexpriority ept vpid ept_ad fsgsb
                         ase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid rdseed adx smap clflushopt cl
                         wb intel_pt sha_ni xsaveopt xsavec xgetbv1 xsaves split_lock_detect user_shst
                         k avx_vnni dtherm ida arat pln pts hwp hwp_notify hwp_act_window hwp_epp hwp_
                         pkg_req hfi vnmi umip pku ospke waitpkg gfni vaes vpclmulqdq tme rdpid movdir
                         i movdir64b fsrm md_clear serialize pconfig arch_lbr ibt flush_l1d arch_capab
                         ilities
Virtualization features: 
  Virtualization:        VT-x
Caches (sum of all):     
  L1d:                   640 KiB (16 instances)
  L1i:                   768 KiB (16 instances)
  L2:                    14 MiB (10 instances)
  L3:                    30 MiB (1 instance)
NUMA:                    
  NUMA node(s):          1
  NUMA node0 CPU(s):     0-23
Vulnerabilities:         
  Gather data sampling:  Not affected
  Itlb multihit:         Not affected
  L1tf:                  Not affected
  Mds:                   Not affected
  Meltdown:              Not affected
  Mmio stale data:       Not affected
  Retbleed:              Not affected
  Spec rstack overflow:  Not affected
  Spec store bypass:     Mitigation; Speculative Store Bypass disabled via prctl
  Spectre v1:            Mitigation; usercopy/swapgs barriers and __user pointer sanitization
  Spectre v2:            Mitigation; Enhanced / Automatic IBRS, IBPB conditional, RSB filling, PBRSB-e
                         IBRS SW sequence
  Srbds:                 Not affected
  Tsx async abort:       Not affected
```

free -mh:

```
               total        used        free      shared  buff/cache   available
Mem:            62Gi        18Gi        20Gi       2.3Gi        26Gi        44Gi
Swap:           95Gi          0B        95Gi
```

## Sequential

```
Running test for n=10, seed=1
Results for n=10:
Max time: .003
Min time: .001
Avg time: 0.001
Median time: 0.001
--------------------------------------------------
Running test for n=20, seed=1
Results for n=20:
Max time: .004
Min time: .002
Avg time: 0.004
Median time: 0.003
--------------------------------------------------
Running test for n=50, seed=1
Results for n=50:
Max time: .016
Min time: .006
Avg time: 0.009
Median time: 0.0105
--------------------------------------------------
Running test for n=100, seed=1
Results for n=100:
Max time: .045
Min time: .033
Avg time: 0.038
Median time: 0.0365
--------------------------------------------------
Running test for n=200, seed=1
Results for n=200:
Max time: .502
Min time: .494
Avg time: 0.499
Median time: 0.498
--------------------------------------------------
Running test for n=400, seed=1
Results for n=400:
Max time: 7.947
Min time: 7.892
Avg time: 7.932
Median time: 7.929
--------------------------------------------------
```

## Parallel (MPI + OpenMP)

Used 10 MPI processes, which results in oversubscription when number of threads is greater than 2.

```
Running test for n=10, seed=1, thread_count=1
Results for n=10, thread_count=1:
Max time: .265
Min time: .225
Avg time: 0.253
Median time: 0.2495
--------------------------------------------------
Running test for n=10, seed=1, thread_count=2
Results for n=10, thread_count=2:
Max time: .279
Min time: .230
Avg time: 0.23
Median time: 0.2485
--------------------------------------------------
Running test for n=10, seed=1, thread_count=4
Results for n=10, thread_count=4:
Max time: .295
Min time: .259
Avg time: 0.273
Median time: 0.276
--------------------------------------------------
Running test for n=10, seed=1, thread_count=8
Results for n=10, thread_count=8:
Max time: .329
Min time: .282
Avg time: 0.309
Median time: 0.3085
--------------------------------------------------
Running test for n=20, seed=1, thread_count=1
Results for n=20, thread_count=1:
Max time: .255
Min time: .217
Avg time: 0.247
Median time: 0.242
--------------------------------------------------
Running test for n=20, seed=1, thread_count=2
Results for n=20, thread_count=2:
Max time: .289
Min time: .236
Avg time: 0.289
Median time: 0.269
--------------------------------------------------
Running test for n=20, seed=1, thread_count=4
Results for n=20, thread_count=4:
Max time: .287
Min time: .243
Avg time: 0.251
Median time: 0.2665
--------------------------------------------------
Running test for n=20, seed=1, thread_count=8
Results for n=20, thread_count=8:
Max time: .350
Min time: .271
Avg time: 0.304
Median time: 0.318
--------------------------------------------------
Running test for n=50, seed=1, thread_count=1
Results for n=50, thread_count=1:
Max time: .287
Min time: .218
Avg time: 0.263
Median time: 0.257
--------------------------------------------------
Running test for n=50, seed=1, thread_count=2
Results for n=50, thread_count=2:
Max time: .266
Min time: .221
Avg time: 0.255
Median time: 0.252
--------------------------------------------------
Running test for n=50, seed=1, thread_count=4
Results for n=50, thread_count=4:
Max time: .310
Min time: .231
Avg time: 0.281
Median time: 0.272
--------------------------------------------------
Running test for n=50, seed=1, thread_count=8
Results for n=50, thread_count=8:
Max time: .336
Min time: .289
Avg time: 0.319
Median time: 0.3095
--------------------------------------------------
Running test for n=100, seed=1, thread_count=1
Results for n=100, thread_count=1:
Max time: .329
Min time: .247
Avg time: 0.249
Median time: 0.2555
--------------------------------------------------
Running test for n=100, seed=1, thread_count=2
Results for n=100, thread_count=2:
Max time: .266
Min time: .218
Avg time: 0.236
Median time: 0.2475
--------------------------------------------------
Running test for n=100, seed=1, thread_count=4
Results for n=100, thread_count=4:
Max time: .298
Min time: .255
Avg time: 0.269
Median time: 0.269
--------------------------------------------------
Running test for n=100, seed=1, thread_count=8
Results for n=100, thread_count=8:
Max time: .356
Min time: .295
Avg time: 0.295
Median time: 0.316
--------------------------------------------------
Running test for n=200, seed=1, thread_count=1
Results for n=200, thread_count=1:
Max time: .348
Min time: .303
Avg time: 0.345
Median time: 0.3315
--------------------------------------------------
Running test for n=200, seed=1, thread_count=2
Results for n=200, thread_count=2:
Max time: .337
Min time: .292
Avg time: 0.335
Median time: 0.307
--------------------------------------------------
Running test for n=200, seed=1, thread_count=4
Results for n=200, thread_count=4:
Max time: .363
Min time: .313
Avg time: 0.327
Median time: 0.327
--------------------------------------------------
Running test for n=200, seed=1, thread_count=8
Results for n=200, thread_count=8:
Max time: .438
Min time: .394
Avg time: 0.405
Median time: 0.4225
--------------------------------------------------
Running test for n=400, seed=1, thread_count=1
Results for n=400, thread_count=1:
Max time: 1.475
Min time: 1.330
Avg time: 1.336
Median time: 1.353
--------------------------------------------------
Running test for n=400, seed=1, thread_count=2
Results for n=400, thread_count=2:
Max time: .899
Min time: .778
Avg time: 0.845
Median time: 0.8565
--------------------------------------------------
Running test for n=400, seed=1, thread_count=4
Results for n=400, thread_count=4:
Max time: .927
Min time: .821
Avg time: 0.891
Median time: 0.8745
--------------------------------------------------
Running test for n=400, seed=1, thread_count=8
Results for n=400, thread_count=8:
Max time: 1.218
Min time: 1.120
Avg time: 1.155
Median time: 1.1535
--------------------------------------------------
```