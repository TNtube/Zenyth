#define Default_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
    "DescriptorTable(CBV(b0, numDescriptors = 1), visibility=SHADER_VISIBILITY_VERTEX), " \
    "DescriptorTable(CBV(b1, numDescriptors = 1), visibility=SHADER_VISIBILITY_VERTEX), " \
    "DescriptorTable(CBV(b2, numDescriptors = 1), visibility=SHADER_VISIBILITY_PIXEL), " \
    "DescriptorTable(SRV(t0, numDescriptors = 1), visibility=SHADER_VISIBILITY_PIXEL), " \
    "DescriptorTable(SRV(t1, numDescriptors = 1), visibility=SHADER_VISIBILITY_PIXEL), " \
    "DescriptorTable(SRV(t2, numDescriptors = 1), visibility=SHADER_VISIBILITY_PIXEL), " \
    "DescriptorTable(SRV(t3, numDescriptors = 1), visibility=SHADER_VISIBILITY_PIXEL), " \
    "StaticSampler(s0, " \
        "addressU = TEXTURE_ADDRESS_WRAP, " \
        "addressV = TEXTURE_ADDRESS_WRAP, " \
        "addressW = TEXTURE_ADDRESS_WRAP, " \
        "filter = FILTER_MIN_MAG_MIP_LINEAR, " \
        "visibility=SHADER_VISIBILITY_PIXEL)"