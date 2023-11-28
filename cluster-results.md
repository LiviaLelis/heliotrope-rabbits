# Cluster Results

Node Count: 10

LSCPU:

```
Architecture:            x86_64
  CPU op-mode(s):        32-bit, 64-bit
  Address sizes:         39 bits physical, 48 bits virtual
  Byte Order:            Little Endian
CPU(s):                  8
  On-line CPU(s) list:   0-7
Vendor ID:               GenuineIntel
  Model name:            Intel(R) Core(TM) i7-4790 CPU @ 3.60GHz
    CPU family:          6
    Model:               60
    Thread(s) per core:  2
    Core(s) per socket:  4
    Socket(s):           1
    Stepping:            3
    CPU max MHz:         4000.0000
    CPU min MHz:         800.0000
    BogoMIPS:            7199.99
    Flags:               fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse
                          sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc arch_perfmon pebs bts rep_good nopl xtop
                         ology nonstop_tsc cpuid aperfmperf pni pclmulqdq dtes64 monitor ds_cpl vmx smx est tm2 ssse3 sdbg fma
                          cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand l
                         ahf_lm abm cpuid_fault invpcid_single pti ssbd ibrs ibpb stibp tpr_shadow vnmi flexpriority ept vpid 
                         ept_ad fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid xsaveopt dtherm ida arat pln pts md_clear
                          flush_l1d
Virtualization features: 
  Virtualization:        VT-x
Caches (sum of all):     
  L1d:                   128 KiB (4 instances)
  L1i:                   128 KiB (4 instances)
  L2:                    1 MiB (4 instances)
  L3:                    8 MiB (1 instance)
NUMA:                    
  NUMA node(s):          1
  NUMA node0 CPU(s):     0-7
Vulnerabilities:         
  Gather data sampling:  Not affected
  Itlb multihit:         KVM: Mitigation: Split huge pages
  L1tf:                  Mitigation; PTE Inversion; VMX conditional cache flushes, SMT vulnerable
  Mds:                   Mitigation; Clear CPU buffers; SMT vulnerable
  Meltdown:              Mitigation; PTI
  Mmio stale data:       Unknown: No mitigations
  Retbleed:              Not affected
  Spec rstack overflow:  Not affected
  Spec store bypass:     Mitigation; Speculative Store Bypass disabled via prctl and seccomp
  Spectre v1:            Mitigation; usercopy/swapgs barriers and __user pointer sanitization
  Spectre v2:            Mitigation; Retpolines, IBPB conditional, IBRS_FW, STIBP conditional, RSB filling, PBRSB-eIBRS Not af
                         fected
  Srbds:                 Mitigation; Microcode
  Tsx async abort:       Not affected
```

free -mh:

```
               total        used        free      shared  buff/cache   available
Mem:            31Gi       7.3Gi       6.9Gi       2.0Mi        17Gi        23Gi
Swap:          8.0Gi       0.0Ki       8.0Gi
```

## Sequential

```
Running test for n=10, seed=1
Results for n=10:
Max time: .002
Min time: .001
Avg time: 0.002
Median time: 0.001
--------------------------------------------------
Running test for n=20, seed=1
Results for n=20:
Max time: .001
Min time: .001
Avg time: 0.001
Median time: 0.001
--------------------------------------------------
Running test for n=50, seed=1
Results for n=50:
Max time: .006
Min time: .006
Avg time: 0.006
Median time: 0.006
--------------------------------------------------
Running test for n=100, seed=1
Results for n=100:
Max time: .091
Min time: .089
Avg time: 0.09
Median time: 0.09
--------------------------------------------------
Running test for n=200, seed=1
Results for n=200:
Max time: 1.420
Min time: 1.411
Avg time: 1.417
Median time: 1.417
--------------------------------------------------
Running test for n=400, seed=1
Results for n=400:
Max time: 22.631
Min time: 22.573
Avg time: 22.621
Median time: 22.6175
--------------------------------------------------
```

## Parallel (MPI + OpenMP)

```
Running test for n=10, seed=1, thread_count=1
Results for n=10, thread_count=1:
Max time: 2.846
Min time: .451
Avg time: 2.846
Median time: 0.516
--------------------------------------------------
Running test for n=10, seed=1, thread_count=2
Results for n=10, thread_count=2:
Max time: 1.346
Min time: .445
Avg time: 1.241
Median time: 0.4775
--------------------------------------------------
Running test for n=10, seed=1, thread_count=4
Results for n=10, thread_count=4:
Max time: 1.286
Min time: .483
Avg time: 0.537
Median time: 0.5325
--------------------------------------------------
Running test for n=10, seed=1, thread_count=8
Results for n=10, thread_count=8:
Max time: .910
Min time: .498
Avg time: 0.529
Median time: 0.582
--------------------------------------------------
Running test for n=20, seed=1, thread_count=1
Results for n=20, thread_count=1:
Max time: 1.991
Min time: .471
Avg time: 0.488
Median time: 0.4905
--------------------------------------------------
Running test for n=20, seed=1, thread_count=2
Results for n=20, thread_count=2:
Max time: 1.279
Min time: .444
Avg time: 0.444
Median time: 0.477
--------------------------------------------------
Running test for n=20, seed=1, thread_count=4
Results for n=20, thread_count=4:
Max time: .796
Min time: .480
Avg time: 0.539
Median time: 0.5645
--------------------------------------------------
Running test for n=20, seed=1, thread_count=8
Results for n=20, thread_count=8:
Max time: 6.528
Min time: .534
Avg time: 1.394
Median time: 0.6575
--------------------------------------------------
Running test for n=50, seed=1, thread_count=1
Results for n=50, thread_count=1:
Max time: .942
Min time: .474
Avg time: 0.743
Median time: 0.7115
--------------------------------------------------
Running test for n=50, seed=1, thread_count=2
Results for n=50, thread_count=2:
Max time: 1.087
Min time: .478
Avg time: 0.523
Median time: 0.8375
--------------------------------------------------
Running test for n=50, seed=1, thread_count=4
Results for n=50, thread_count=4:
Max time: 1.320
Min time: .466
Avg time: 0.484
Median time: 0.7155
--------------------------------------------------
Running test for n=50, seed=1, thread_count=8
Results for n=50, thread_count=8:
Max time: 1.242
Min time: .502
Avg time: 0.953
Median time: 0.7425
--------------------------------------------------
Running test for n=100, seed=1, thread_count=1
Results for n=100, thread_count=1:
Max time: 2.081
Min time: .523
Avg time: 1.227
Median time: 0.895
--------------------------------------------------
Running test for n=100, seed=1, thread_count=2
Results for n=100, thread_count=2:
Max time: 4.613
Min time: .500
Avg time: 0.5
Median time: 0.585
--------------------------------------------------
Running test for n=100, seed=1, thread_count=4
Results for n=100, thread_count=4:
Max time: 1.139
Min time: .509
Avg time: 0.964
Median time: 0.565
--------------------------------------------------
Running test for n=100, seed=1, thread_count=8
Results for n=100, thread_count=8:
Max time: 2.247
Min time: .618
Avg time: 0.939
Median time: 0.8435
--------------------------------------------------
Running test for n=200, seed=1, thread_count=1
Results for n=200, thread_count=1:
Max time: 5.157
Min time: 1.372
Avg time: 1.694
Median time: 1.7415
--------------------------------------------------
Running test for n=200, seed=1, thread_count=2
Results for n=200, thread_count=2:
Max time: 2.112
Min time: .988
Avg time: 1.123
Median time: 1.216
--------------------------------------------------
Running test for n=200, seed=1, thread_count=4
Results for n=200, thread_count=4:
Max time: 3.844
Min time: 1.024
Avg time: 1.024
Median time: 1.2485
--------------------------------------------------
Running test for n=200, seed=1, thread_count=8
Results for n=200, thread_count=8:
Max time: 1.421
Min time: 1.091
Avg time: 1.091
Median time: 1.139
--------------------------------------------------
Running test for n=400, seed=1, thread_count=1
Results for n=400, thread_count=1:
Max time: 20.681
Min time: 15.183
Avg time: 15.183
Median time: 17.9375
--------------------------------------------------
Running test for n=400, seed=1, thread_count=2
Results for n=400, thread_count=2:
Max time: 12.665
Min time: 9.115
Avg time: 10.863
Median time: 10.717
--------------------------------------------------
Running test for n=400, seed=1, thread_count=4
Results for n=400, thread_count=4:
Max time: 11.689
Min time: 8.425
Avg time: 8.668
Median time: 9.8265
--------------------------------------------------
Running test for n=400, seed=1, thread_count=8
Results for n=400, thread_count=8:
Max time: 9.806
Min time: 7.889
Avg time: 8.019
Median time: 8.894
--------------------------------------------------
```