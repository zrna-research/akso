# 1 "../../external/CMSIS//DSP_Lib/Source/TransformFunctions/arm_bitreversal2.S"
# 1 "/home/nicolas/g/Source/akso-sub/axoloti/firmware//"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "../../external/CMSIS//DSP_Lib/Source/TransformFunctions/arm_bitreversal2.S"
;
# 13 "../../external/CMSIS//DSP_Lib/Source/TransformFunctions/arm_bitreversal2.S"
;
# 65 "../../external/CMSIS//DSP_Lib/Source/TransformFunctions/arm_bitreversal2.S"
    .syntax unified


 .section .text
 .thumb

;






 .global arm_bitreversal_32
 .global arm_bitreversal_16






 .type arm_bitreversal_16, %function
 .type arm_bitreversal_32, %function
# 142 "../../external/CMSIS//DSP_Lib/Source/TransformFunctions/arm_bitreversal2.S"
arm_bitreversal_32 :
 ADDS r3,r1,#1
 CMP r3,#1
 IT LS
 BXLS lr
 PUSH {r4-r9}
 ADDS r1,r2,#2
 LSRS r3,r3,#2
arm_bitreversal_32_0 : ;
 LDRH r8,[r1,#4]
 LDRH r9,[r1,#2]
 LDRH r2,[r1,#0]
 LDRH r12,[r1,#-2]
 ADD r8,r0,r8
 ADD r9,r0,r9
 ADD r2,r0,r2
 ADD r12,r0,r12
 LDR r7,[r9,#0]
 LDR r6,[r8,#0]
 LDR r5,[r2,#0]
 LDR r4,[r12,#0]
 STR r6,[r9,#0]
 STR r7,[r8,#0]
 STR r5,[r12,#0]
 STR r4,[r2,#0]
 LDR r7,[r9,#4]
 LDR r6,[r8,#4]
 LDR r5,[r2,#4]
 LDR r4,[r12,#4]
 STR r6,[r9,#4]
 STR r7,[r8,#4]
 STR r5,[r12,#4]
 STR r4,[r2,#4]
 ADDS r1,r1,#8
 SUBS r3,r3,#1
 BNE arm_bitreversal_32_0
 POP {r4-r9}
 BX lr


arm_bitreversal_16 :
 ADDS r3,r1,#1
 CMP r3,#1
 IT LS
 BXLS lr
 PUSH {r4-r9}
 ADDS r1,r2,#2
 LSRS r3,r3,#2
arm_bitreversal_16_0 : ;
 LDRH r8,[r1,#4]
 LDRH r9,[r1,#2]
 LDRH r2,[r1,#0]
 LDRH r12,[r1,#-2]
 ADD r8,r0,r8,LSR #1
 ADD r9,r0,r9,LSR #1
 ADD r2,r0,r2,LSR #1
 ADD r12,r0,r12,LSR #1
 LDR r7,[r9,#0]
 LDR r6,[r8,#0]
 LDR r5,[r2,#0]
 LDR r4,[r12,#0]
 STR r6,[r9,#0]
 STR r7,[r8,#0]
 STR r5,[r12,#0]
 STR r4,[r2,#0]
 ADDS r1,r1,#8
 SUBS r3,r3,#1
 BNE arm_bitreversal_16_0
 POP {r4-r9}
 BX lr





