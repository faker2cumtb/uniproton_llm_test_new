T(RN,                  0x0p+0,                  0x1p+0,          0x0p+0, 0)
T(RN,                 -0x0p+0,                  0x1p+0,          0x0p+0, 0)
T(RN,                     inf,                  0x0p+0,          0x0p+0, 0)
T(RN,                    -inf,                  0x0p+0,          0x0p+0, 0)
T(RN,                     nan,                     nan,          0x0p+0, 0)
// hard cases
#if (OS_HARDWARE_PLATFORM != OS_X86_64)
T(RN,   -0x1.33d132fd04a92p+1,   0x1.092b2a541a68ep-19,   -0x1.2ebdbp-2, INEXACT)
T(RN,   -0x1.33d15297be06fp+1,   0x1.5352913be3275p-26,   0x1.c281e2p-7, INEXACT)
T(RN,     0x1.33d152e971b4p+1,  -0x1.19b7921f03c8ep-54,    0x1.ece5dp-5, INEXACT)
T(RN,    0x1.6148f5b2c2e45p+2,  -0x1.fbb40985f6e34p-56,  -0x1.1ac8a8p-2, INEXACT)
T(RN,    0x1.14eb56cccdecap+3,  -0x1.6e8eeb22e5818p-54,  -0x1.63ab44p-2, INEXACT)
#endif
T(RN, 0x1.c071b22fbbafap+1023, -0x1.a348b1f34dd1ap-526,   0x1.3153c2p-6, INEXACT)
#if (OS_HARDWARE_PLATFORM != OS_X86_64)
T(RN,    0x1.f7350b1701ef7p+0,    0x1.f32b3a3640292p-3,  -0x1.b3bad8p-5, INEXACT)
#endif
T(RN,   0x1.f8e4d2e98fb6bp-14,    0x1.ffffffe0e1d1bp-1,    0x1.e1c4cp-2, INEXACT)
T(RN,  -0x1.fd2421ac2a80ap-14,    0x1.ffffffe05b3a7p-1,   0x1.e0f12cp-2, INEXACT)
