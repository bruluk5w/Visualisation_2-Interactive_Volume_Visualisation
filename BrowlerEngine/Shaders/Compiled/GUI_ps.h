#if 0
;
; Input signature:
;
; Name                 Index   Mask Register SysValue  Format   Used
; -------------------- ----- ------ -------- -------- ------- ------
; SV_Position              0   xyzw        0      POS   float       
; COLOR                    0   xyzw        1     NONE   float       
; TEXCOORD                 0   xy          2     NONE   float       
;
;
; Output signature:
;
; Name                 Index   Mask Register SysValue  Format   Used
; -------------------- ----- ------ -------- -------- ------- ------
; SV_Target                0   xyzw        0   TARGET   float   xyzw
;
;
; Pipeline Runtime Information: 
;
; Pixel Shader
; DepthOutput=0
; SampleFrequency=0
;
;
; Input signature:
;
; Name                 Index             InterpMode DynIdx
; -------------------- ----- ---------------------- ------
; SV_Position              0          noperspective       
; COLOR                    0                 linear       
; TEXCOORD                 0                 linear       
;
; Output signature:
;
; Name                 Index             InterpMode DynIdx
; -------------------- ----- ---------------------- ------
; SV_Target                0                              
;
; Buffer Definitions:
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; sampler0                          sampler      NA          NA      S0             s0     1
; texture0                          texture     f32          2d      T0             t0     1
;
;
; ViewId state:
;
; Number of inputs: 10, outputs: 4
; Outputs dependent on ViewId: {  }
; Inputs contributing to computation of Outputs:
;   output 0 depends on inputs: { 4, 8, 9 }
;   output 1 depends on inputs: { 5, 8, 9 }
;   output 2 depends on inputs: { 6, 8, 9 }
;   output 3 depends on inputs: { 7, 8, 9 }
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.SamplerState = type { i32 }
%"class.Texture2D<vector<float, 4> >" = type { <4 x float>, %"class.Texture2D<vector<float, 4> >::mips_type" }
%"class.Texture2D<vector<float, 4> >::mips_type" = type { i32 }
%dx.types.Handle = type { i8* }
%dx.types.ResRet.f32 = type { float, float, float, float, i32 }
%struct.PS_INPUT = type { <4 x float>, <4 x float>, <2 x float> }

@"\01?sampler0@@3USamplerState@@A" = external constant %struct.SamplerState, align 4
@"\01?texture0@@3V?$Texture2D@V?$vector@M$03@@@@A" = external constant %"class.Texture2D<vector<float, 4> >", align 4

define void @main() {
  %texture0_texture_2d = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 0, i1 false)  ; CreateHandle(resourceClass,rangeId,index,nonUniformIndex)
  %sampler0_sampler = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 3, i32 0, i32 0, i1 false)  ; CreateHandle(resourceClass,rangeId,index,nonUniformIndex)
  %1 = call float @dx.op.loadInput.f32(i32 4, i32 2, i32 0, i8 0, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %2 = call float @dx.op.loadInput.f32(i32 4, i32 2, i32 0, i8 1, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %3 = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 0, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %4 = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 1, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %5 = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 2, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %6 = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 3, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %7 = call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60, %dx.types.Handle %texture0_texture_2d, %dx.types.Handle %sampler0_sampler, float %1, float %2, float undef, float undef, i32 undef, i32 undef, i32 undef, float undef)  ; Sample(srv,sampler,coord0,coord1,coord2,coord3,offset0,offset1,offset2,clamp)
  %8 = extractvalue %dx.types.ResRet.f32 %7, 0
  %9 = extractvalue %dx.types.ResRet.f32 %7, 1
  %10 = extractvalue %dx.types.ResRet.f32 %7, 2
  %11 = extractvalue %dx.types.ResRet.f32 %7, 3
  %.i0 = fmul fast float %8, %3
  %.i1 = fmul fast float %9, %4
  %.i2 = fmul fast float %10, %5
  %.i3 = fmul fast float %11, %6
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %.i0)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float %.i1)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float %.i2)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float %.i3)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  ret void
}

; Function Attrs: nounwind readnone
declare float @dx.op.loadInput.f32(i32, i32, i32, i8, i32) #0

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #1

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.f32 @dx.op.sample.f32(i32, %dx.types.Handle, %dx.types.Handle, float, float, float, float, i32, i32, i32, float) #2

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandle(i32, i8, i32, i32, i1) #2

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
attributes #2 = { nounwind readonly }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.resources = !{!4}
!dx.typeAnnotations = !{!10, !20}
!dx.viewIdState = !{!24}
!dx.entryPoints = !{!25}

!0 = !{!"dxc 1.2"}
!1 = !{i32 1, i32 0}
!2 = !{i32 1, i32 4}
!3 = !{!"ps", i32 6, i32 0}
!4 = !{!5, null, null, !8}
!5 = !{!6}
!6 = !{i32 0, %"class.Texture2D<vector<float, 4> >"* undef, !"texture0", i32 0, i32 0, i32 1, i32 2, i32 0, !7}
!7 = !{i32 0, i32 9}
!8 = !{!9}
!9 = !{i32 0, %struct.SamplerState* undef, !"sampler0", i32 0, i32 0, i32 1, i32 0, null}
!10 = !{i32 0, %"class.Texture2D<vector<float, 4> >" undef, !11, %"class.Texture2D<vector<float, 4> >::mips_type" undef, !14, %struct.PS_INPUT undef, !16}
!11 = !{i32 20, !12, !13}
!12 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 9}
!13 = !{i32 6, !"mips", i32 3, i32 16}
!14 = !{i32 4, !15}
!15 = !{i32 6, !"handle", i32 3, i32 0, i32 7, i32 5}
!16 = !{i32 40, !17, !18, !19}
!17 = !{i32 6, !"pos", i32 3, i32 0, i32 4, !"SV_POSITION", i32 7, i32 9}
!18 = !{i32 6, !"col", i32 3, i32 16, i32 4, !"COLOR0", i32 7, i32 9}
!19 = !{i32 6, !"uv", i32 3, i32 32, i32 4, !"TEXCOORD0", i32 7, i32 9}
!20 = !{i32 1, void ()* @main, !21}
!21 = !{!22}
!22 = !{i32 0, !23, !23}
!23 = !{}
!24 = !{[12 x i32] [i32 10, i32 4, i32 0, i32 0, i32 0, i32 0, i32 1, i32 2, i32 4, i32 8, i32 15, i32 15]}
!25 = !{void ()* @main, !"main", !26, !4, null}
!26 = !{!27, !32, null}
!27 = !{!28, !30, !31}
!28 = !{i32 0, !"SV_Position", i8 9, i8 3, !29, i8 4, i32 1, i8 4, i32 0, i8 0, null}
!29 = !{i32 0}
!30 = !{i32 1, !"COLOR", i8 9, i8 0, !29, i8 2, i32 1, i8 4, i32 1, i8 0, null}
!31 = !{i32 2, !"TEXCOORD", i8 9, i8 0, !29, i8 2, i32 1, i8 2, i32 2, i8 0, null}
!32 = !{!33}
!33 = !{i32 0, !"SV_Target", i8 9, i8 16, !29, i8 0, i32 1, i8 4, i32 0, i8 0, null}

#endif

const unsigned char GUI_ps_ps[] = {
  0x44, 0x58, 0x42, 0x43, 0x43, 0xc2, 0x4b, 0xee, 0xd0, 0x72, 0x14, 0xf4,
  0x96, 0xaf, 0x3f, 0xd3, 0x50, 0x94, 0x3b, 0x45, 0x01, 0x00, 0x00, 0x00,
  0x69, 0x0a, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00,
  0x44, 0x00, 0x00, 0x00, 0xcf, 0x00, 0x00, 0x00, 0x09, 0x01, 0x00, 0x00,
  0xf1, 0x01, 0x00, 0x00, 0x53, 0x46, 0x49, 0x30, 0x08, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x53, 0x47, 0x31,
  0x83, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
  0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x53, 0x56, 0x5f, 0x50, 0x6f, 0x73, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x00,
  0x43, 0x4f, 0x4c, 0x4f, 0x52, 0x00, 0x54, 0x45, 0x58, 0x43, 0x4f, 0x4f,
  0x52, 0x44, 0x00, 0x4f, 0x53, 0x47, 0x31, 0x32, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x03,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x53, 0x56, 0x5f, 0x54, 0x61, 0x72, 0x67, 0x65, 0x74,
  0x00, 0x50, 0x53, 0x56, 0x30, 0xe0, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
  0xff, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x00, 0x03, 0x01, 0x00, 0x00,
  0x00, 0x02, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x43, 0x4f,
  0x4c, 0x4f, 0x52, 0x00, 0x54, 0x45, 0x58, 0x43, 0x4f, 0x4f, 0x52, 0x44,
  0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x44,
  0x03, 0x03, 0x04, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x01, 0x01, 0x44, 0x00, 0x03, 0x02, 0x00, 0x00, 0x07, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x42, 0x00, 0x03, 0x02, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x44,
  0x10, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00,
  0x00, 0x0f, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x58, 0x49, 0x4c, 0x70, 0x08, 0x00,
  0x00, 0x60, 0x00, 0x00, 0x00, 0x1c, 0x02, 0x00, 0x00, 0x44, 0x58, 0x49,
  0x4c, 0x00, 0x01, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x58, 0x08, 0x00,
  0x00, 0x42, 0x43, 0xc0, 0xde, 0x21, 0x0c, 0x00, 0x00, 0x13, 0x02, 0x00,
  0x00, 0x0b, 0x82, 0x20, 0x00, 0x02, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00,
  0x00, 0x07, 0x81, 0x23, 0x91, 0x41, 0xc8, 0x04, 0x49, 0x06, 0x10, 0x32,
  0x39, 0x92, 0x01, 0x84, 0x0c, 0x25, 0x05, 0x08, 0x19, 0x1e, 0x04, 0x8b,
  0x62, 0x80, 0x14, 0x45, 0x02, 0x42, 0x92, 0x0b, 0x42, 0xa4, 0x10, 0x32,
  0x14, 0x38, 0x08, 0x18, 0x4b, 0x0a, 0x32, 0x52, 0x88, 0x48, 0x90, 0x14,
  0x20, 0x43, 0x46, 0x88, 0xa5, 0x00, 0x19, 0x32, 0x42, 0xe4, 0x48, 0x0e,
  0x90, 0x91, 0x22, 0xc4, 0x50, 0x41, 0x51, 0x81, 0x8c, 0xe1, 0x83, 0xe5,
  0x8a, 0x04, 0x29, 0x46, 0x06, 0x51, 0x18, 0x00, 0x00, 0x08, 0x00, 0x00,
  0x00, 0x1b, 0x8c, 0xe0, 0xff, 0xff, 0xff, 0xff, 0x07, 0x40, 0x02, 0xa8,
  0x0d, 0x84, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x03, 0x20, 0x6d, 0x30, 0x86,
  0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x09, 0xa8, 0x00, 0x49, 0x18, 0x00,
  0x00, 0x03, 0x00, 0x00, 0x00, 0x13, 0x82, 0x60, 0x42, 0x20, 0x4c, 0x08,
  0x06, 0x00, 0x00, 0x00, 0x00, 0x89, 0x20, 0x00, 0x00, 0x48, 0x00, 0x00,
  0x00, 0x32, 0x22, 0x48, 0x09, 0x20, 0x64, 0x85, 0x04, 0x93, 0x22, 0xa4,
  0x84, 0x04, 0x93, 0x22, 0xe3, 0x84, 0xa1, 0x90, 0x14, 0x12, 0x4c, 0x8a,
  0x8c, 0x0b, 0x84, 0xa4, 0x4c, 0x10, 0x70, 0x73, 0x04, 0x60, 0x70, 0x93,
  0x34, 0x45, 0x94, 0x30, 0xf9, 0x2c, 0xc0, 0x3c, 0x0b, 0x11, 0xb1, 0x13,
  0x30, 0x11, 0x28, 0x00, 0x14, 0x66, 0x00, 0x86, 0x11, 0x88, 0x61, 0xa6,
  0x36, 0x18, 0x07, 0x76, 0x08, 0x87, 0x79, 0x98, 0x07, 0x37, 0xa0, 0x85,
  0x72, 0xc0, 0x07, 0x7a, 0xa8, 0x07, 0x79, 0x28, 0x07, 0x39, 0x20, 0x05,
  0x3e, 0xb0, 0x87, 0x72, 0x18, 0x07, 0x7a, 0x78, 0x07, 0x79, 0xe0, 0x03,
  0x73, 0x60, 0x87, 0x77, 0x08, 0x07, 0x7a, 0x60, 0x03, 0x30, 0xa0, 0x03,
  0x3f, 0x00, 0x03, 0x3f, 0xd0, 0x03, 0x3d, 0x68, 0x87, 0x74, 0x80, 0x87,
  0x79, 0xf8, 0x05, 0x7a, 0xc8, 0x07, 0x78, 0x28, 0x07, 0x14, 0x80, 0x99,
  0xc4, 0x60, 0x1c, 0xd8, 0x21, 0x1c, 0xe6, 0x61, 0x1e, 0xdc, 0x80, 0x16,
  0xca, 0x01, 0x1f, 0xe8, 0xa1, 0x1e, 0xe4, 0xa1, 0x1c, 0xe4, 0x80, 0x14,
  0xf8, 0xc0, 0x1e, 0xca, 0x61, 0x1c, 0xe8, 0xe1, 0x1d, 0xe4, 0x81, 0x0f,
  0xcc, 0x81, 0x1d, 0xde, 0x21, 0x1c, 0xe8, 0x81, 0x0d, 0xc0, 0x80, 0x0e,
  0xfc, 0x00, 0x0c, 0xfc, 0x00, 0x09, 0xa4, 0x90, 0x19, 0x01, 0x28, 0x01,
  0xa2, 0x34, 0x47, 0x80, 0x14, 0x63, 0x00, 0x00, 0x16, 0x20, 0x56, 0x0c,
  0x04, 0x00, 0x58, 0x83, 0xdc, 0x4d, 0xc3, 0xe5, 0x4f, 0xd8, 0x43, 0x48,
  0xfe, 0x4a, 0x48, 0x2b, 0x31, 0xf9, 0xc5, 0x6d, 0xa3, 0x62, 0x8c, 0x31,
  0x00, 0xad, 0x7b, 0x86, 0xcb, 0x9f, 0xb0, 0x87, 0x90, 0xfc, 0x10, 0x68,
  0x86, 0x85, 0x40, 0x41, 0x2c, 0x0c, 0x04, 0x52, 0x8e, 0x31, 0x06, 0x00,
  0x60, 0xd0, 0x9c, 0x23, 0x08, 0x8a, 0x21, 0xc1, 0x02, 0xa0, 0x92, 0x1d,
  0x08, 0x18, 0x46, 0x10, 0x86, 0x7b, 0xa4, 0x29, 0xa2, 0x84, 0xc9, 0x4f,
  0xd9, 0x5f, 0x3c, 0x53, 0xb7, 0x45, 0x03, 0x21, 0x39, 0x19, 0x00, 0x00,
  0x00, 0x13, 0x14, 0x72, 0xc0, 0x87, 0x74, 0x60, 0x87, 0x36, 0x68, 0x87,
  0x79, 0x68, 0x03, 0x72, 0xc0, 0x87, 0x0d, 0xaf, 0x50, 0x0e, 0x6d, 0xd0,
  0x0e, 0x7a, 0x50, 0x0e, 0x6d, 0x00, 0x0f, 0x7a, 0x30, 0x07, 0x72, 0xa0,
  0x07, 0x73, 0x20, 0x07, 0x6d, 0x90, 0x0e, 0x71, 0xa0, 0x07, 0x73, 0x20,
  0x07, 0x6d, 0x90, 0x0e, 0x78, 0xa0, 0x07, 0x73, 0x20, 0x07, 0x6d, 0x90,
  0x0e, 0x71, 0x60, 0x07, 0x7a, 0x30, 0x07, 0x72, 0xd0, 0x06, 0xe9, 0x30,
  0x07, 0x72, 0xa0, 0x07, 0x73, 0x20, 0x07, 0x6d, 0x90, 0x0e, 0x76, 0x40,
  0x07, 0x7a, 0x60, 0x07, 0x74, 0xd0, 0x06, 0xe6, 0x10, 0x07, 0x76, 0xa0,
  0x07, 0x73, 0x20, 0x07, 0x6d, 0x60, 0x0e, 0x73, 0x20, 0x07, 0x7a, 0x30,
  0x07, 0x72, 0xd0, 0x06, 0xe6, 0x60, 0x07, 0x74, 0xa0, 0x07, 0x76, 0x40,
  0x07, 0x6d, 0xe0, 0x0e, 0x78, 0xa0, 0x07, 0x71, 0x60, 0x07, 0x7a, 0x30,
  0x07, 0x72, 0xa0, 0x07, 0x76, 0x40, 0x07, 0x3a, 0x0f, 0x64, 0x90, 0x21,
  0x23, 0x45, 0x44, 0x00, 0x66, 0x00, 0xc0, 0xf4, 0x00, 0x80, 0x87, 0x3c,
  0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c,
  0x79, 0x18, 0x20, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x18, 0xf2, 0x38, 0x40, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x30, 0xe4, 0x99, 0x80, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x60, 0xc8, 0x63, 0x01, 0x01, 0x30, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x40, 0x16, 0x08, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00,
  0x00, 0x32, 0x1e, 0x98, 0x18, 0x19, 0x11, 0x4c, 0x90, 0x8c, 0x09, 0x26,
  0x47, 0xc6, 0x04, 0x43, 0x02, 0x25, 0x30, 0x02, 0x50, 0x08, 0xc5, 0x50,
  0x04, 0x25, 0x51, 0x28, 0x65, 0x50, 0x0e, 0x05, 0x52, 0x0a, 0x05, 0x55,
  0x40, 0xb4, 0x0a, 0xa4, 0x08, 0x46, 0x00, 0x0a, 0xa1, 0x24, 0xca, 0x80,
  0xce, 0x0c, 0x00, 0x99, 0x19, 0x00, 0x2a, 0x33, 0x00, 0x24, 0x66, 0x00,
  0x48, 0xcf, 0x00, 0xd0, 0x1e, 0x8b, 0x51, 0x08, 0x00, 0x00, 0x80, 0x40,
  0x20, 0x90, 0xe7, 0x01, 0x00, 0x79, 0x18, 0x00, 0x00, 0x92, 0x00, 0x00,
  0x00, 0x1a, 0x03, 0x4c, 0x90, 0x46, 0x02, 0x13, 0xc4, 0x83, 0x0c, 0x6f,
  0x0c, 0x24, 0xc6, 0x45, 0x66, 0x43, 0x10, 0x4c, 0x10, 0x80, 0x63, 0x82,
  0x00, 0x20, 0x1b, 0x84, 0x81, 0x98, 0x20, 0x00, 0xc9, 0x06, 0x61, 0x30,
  0x28, 0xc0, 0xcd, 0x4d, 0x10, 0x00, 0x65, 0xc3, 0x80, 0x24, 0xc4, 0x04,
  0xe1, 0xd0, 0x88, 0xd0, 0x95, 0xe1, 0xd1, 0xd5, 0xc9, 0x95, 0xc1, 0x4c,
  0x10, 0x80, 0x65, 0x82, 0x00, 0x30, 0x1b, 0x04, 0xc2, 0xd9, 0x90, 0x10,
  0x0b, 0x43, 0x10, 0x43, 0x43, 0x3c, 0x1b, 0x02, 0x68, 0x82, 0x20, 0x74,
  0x44, 0xe6, 0xc2, 0xda, 0xe0, 0xd8, 0xca, 0xe4, 0x60, 0x36, 0x20, 0x84,
  0x34, 0x11, 0xc4, 0x40, 0x00, 0x1b, 0x02, 0x6a, 0x03, 0x11, 0x01, 0x40,
  0x35, 0x41, 0x30, 0xb6, 0x09, 0x02, 0xd0, 0x30, 0x40, 0x9b, 0x20, 0x00,
  0xce, 0x04, 0x01, 0x78, 0x36, 0x18, 0x49, 0xa6, 0x11, 0x9b, 0x43, 0xa2,
  0x2d, 0x0d, 0x6e, 0x6e, 0x82, 0x00, 0x40, 0x1b, 0x88, 0xa4, 0xd3, 0xbc,
  0x0d, 0x03, 0xc6, 0x7d, 0x13, 0x84, 0x82, 0xa3, 0x81, 0x16, 0xe6, 0x46,
  0xc6, 0x56, 0x36, 0x41, 0x00, 0xa2, 0x0d, 0x46, 0x22, 0x06, 0x1a, 0xb1,
  0x8d, 0xc1, 0x06, 0xc1, 0x20, 0x83, 0x09, 0x82, 0xe6, 0x4d, 0x10, 0x00,
  0x89, 0x03, 0xdc, 0xdb, 0x1c, 0x97, 0x29, 0xab, 0x2f, 0xa8, 0xa7, 0xa9,
  0x24, 0xaa, 0xa4, 0x27, 0xa7, 0x0d, 0x48, 0x82, 0x06, 0x1a, 0x61, 0xa4,
  0xc1, 0xe6, 0x70, 0x18, 0x7b, 0x63, 0xa3, 0x31, 0xf4, 0xc4, 0xf4, 0x24,
  0x05, 0xb3, 0x01, 0x49, 0xd6, 0x40, 0xf3, 0x0c, 0x36, 0xd8, 0x1c, 0x0a,
  0x75, 0x76, 0x13, 0x04, 0x60, 0x62, 0x42, 0x55, 0x84, 0x35, 0xf4, 0xf4,
  0x24, 0x45, 0x04, 0xb3, 0x01, 0x49, 0xdc, 0x40, 0x7b, 0x03, 0x03, 0x0e,
  0x36, 0x67, 0x03, 0x71, 0x06, 0x6a, 0xd0, 0x06, 0x71, 0xb0, 0xe1, 0x20,
  0x2e, 0x30, 0x08, 0x83, 0x32, 0x30, 0x03, 0x39, 0x98, 0x20, 0x28, 0xc2,
  0x06, 0x60, 0xc3, 0x40, 0xd4, 0x41, 0x1d, 0x6c, 0x08, 0xec, 0x60, 0xc3,
  0x30, 0xd0, 0xc1, 0x1d, 0x4c, 0x10, 0xb6, 0x6f, 0x43, 0x90, 0x07, 0x24,
  0xda, 0xc2, 0xd2, 0xdc, 0xb8, 0x4c, 0x59, 0x7d, 0x41, 0xbd, 0xcd, 0xa5,
  0xd1, 0xa5, 0xbd, 0xb9, 0x4d, 0x10, 0x16, 0x6c, 0x82, 0xb0, 0x64, 0x1b,
  0x02, 0x62, 0x82, 0xb0, 0x5c, 0x13, 0x84, 0xc5, 0xda, 0xb0, 0x10, 0x7c,
  0xd0, 0x07, 0x7e, 0xf0, 0x07, 0xa0, 0x30, 0x80, 0x02, 0x11, 0x0a, 0x00,
  0x8b, 0xa1, 0x27, 0xa6, 0x27, 0xa9, 0x09, 0xc2, 0x52, 0x6d, 0x58, 0x86,
  0x51, 0xe8, 0x83, 0x50, 0xf8, 0x03, 0x52, 0x18, 0x40, 0x61, 0x08, 0x05,
  0x80, 0x08, 0x55, 0x11, 0xd6, 0xd0, 0xd3, 0x93, 0x14, 0xd1, 0x86, 0xa5,
  0x31, 0x85, 0x3e, 0x08, 0x85, 0x3f, 0x20, 0x85, 0x81, 0x14, 0x9a, 0x50,
  0x00, 0x36, 0x0c, 0xa2, 0x50, 0x0a, 0xa7, 0xc0, 0x64, 0xca, 0xea, 0x8b,
  0x2a, 0x4c, 0xee, 0xac, 0x8c, 0x6e, 0x82, 0xb0, 0x50, 0x1b, 0x16, 0x22,
  0x15, 0xfa, 0x40, 0x15, 0xfe, 0x20, 0x14, 0x06, 0x50, 0x20, 0x42, 0x01,
  0xd8, 0x10, 0xac, 0xc2, 0x86, 0x01, 0x15, 0x58, 0x01, 0xd8, 0x50, 0xd0,
  0xc1, 0x1e, 0xb4, 0x82, 0x05, 0x54, 0x61, 0x63, 0xb3, 0x6b, 0x73, 0x49,
  0x23, 0x2b, 0x73, 0xa3, 0x9b, 0x12, 0x04, 0x55, 0xc8, 0xf0, 0x5c, 0xec,
  0xca, 0xe4, 0xe6, 0xd2, 0xde, 0xdc, 0xa6, 0x04, 0x44, 0x13, 0x32, 0x3c,
  0x17, 0xbb, 0x30, 0x36, 0xbb, 0x32, 0xb9, 0x29, 0x81, 0x51, 0x87, 0x0c,
  0xcf, 0x65, 0x0e, 0x2d, 0x8c, 0xac, 0x4c, 0xae, 0xe9, 0x8d, 0xac, 0x8c,
  0x6d, 0x4a, 0x90, 0x94, 0x21, 0xc3, 0x73, 0x91, 0x2b, 0x9b, 0x7b, 0xab,
  0x93, 0x1b, 0x2b, 0x9b, 0x9b, 0x12, 0x54, 0x95, 0xc8, 0xf0, 0x5c, 0xe8,
  0xf2, 0xe0, 0xca, 0x82, 0xdc, 0xdc, 0xde, 0xe8, 0xc2, 0xe8, 0xd2, 0xde,
  0xdc, 0xe6, 0xa6, 0x08, 0x72, 0x70, 0x07, 0x75, 0xc8, 0xf0, 0x5c, 0xec,
  0xd2, 0xca, 0xee, 0x92, 0xc8, 0xa6, 0xe8, 0xc2, 0xe8, 0xca, 0xa6, 0x04,
  0x79, 0x50, 0x87, 0x0c, 0xcf, 0xa5, 0xcc, 0x8d, 0x4e, 0x2e, 0x0f, 0xea,
  0x2d, 0xcd, 0x8d, 0x6e, 0x6e, 0x4a, 0xd0, 0x0a, 0x00, 0x79, 0x18, 0x00,
  0x00, 0x4c, 0x00, 0x00, 0x00, 0x33, 0x08, 0x80, 0x1c, 0xc4, 0xe1, 0x1c,
  0x66, 0x14, 0x01, 0x3d, 0x88, 0x43, 0x38, 0x84, 0xc3, 0x8c, 0x42, 0x80,
  0x07, 0x79, 0x78, 0x07, 0x73, 0x98, 0x71, 0x0c, 0xe6, 0x00, 0x0f, 0xed,
  0x10, 0x0e, 0xf4, 0x80, 0x0e, 0x33, 0x0c, 0x42, 0x1e, 0xc2, 0xc1, 0x1d,
  0xce, 0xa1, 0x1c, 0x66, 0x30, 0x05, 0x3d, 0x88, 0x43, 0x38, 0x84, 0x83,
  0x1b, 0xcc, 0x03, 0x3d, 0xc8, 0x43, 0x3d, 0x8c, 0x03, 0x3d, 0xcc, 0x78,
  0x8c, 0x74, 0x70, 0x07, 0x7b, 0x08, 0x07, 0x79, 0x48, 0x87, 0x70, 0x70,
  0x07, 0x7a, 0x70, 0x03, 0x76, 0x78, 0x87, 0x70, 0x20, 0x87, 0x19, 0xcc,
  0x11, 0x0e, 0xec, 0x90, 0x0e, 0xe1, 0x30, 0x0f, 0x6e, 0x30, 0x0f, 0xe3,
  0xf0, 0x0e, 0xf0, 0x50, 0x0e, 0x33, 0x10, 0xc4, 0x1d, 0xde, 0x21, 0x1c,
  0xd8, 0x21, 0x1d, 0xc2, 0x61, 0x1e, 0x66, 0x30, 0x89, 0x3b, 0xbc, 0x83,
  0x3b, 0xd0, 0x43, 0x39, 0xb4, 0x03, 0x3c, 0xbc, 0x83, 0x3c, 0x84, 0x03,
  0x3b, 0xcc, 0xf0, 0x14, 0x76, 0x60, 0x07, 0x7b, 0x68, 0x07, 0x37, 0x68,
  0x87, 0x72, 0x68, 0x07, 0x37, 0x80, 0x87, 0x70, 0x90, 0x87, 0x70, 0x60,
  0x07, 0x76, 0x28, 0x07, 0x76, 0xf8, 0x05, 0x76, 0x78, 0x87, 0x77, 0x80,
  0x87, 0x5f, 0x08, 0x87, 0x71, 0x18, 0x87, 0x72, 0x98, 0x87, 0x79, 0x98,
  0x81, 0x2c, 0xee, 0xf0, 0x0e, 0xee, 0xe0, 0x0e, 0xf5, 0xc0, 0x0e, 0xec,
  0x30, 0x03, 0x62, 0xc8, 0xa1, 0x1c, 0xe4, 0xa1, 0x1c, 0xcc, 0xa1, 0x1c,
  0xe4, 0xa1, 0x1c, 0xdc, 0x61, 0x1c, 0xca, 0x21, 0x1c, 0xc4, 0x81, 0x1d,
  0xca, 0x61, 0x06, 0xd6, 0x90, 0x43, 0x39, 0xc8, 0x43, 0x39, 0x98, 0x43,
  0x39, 0xc8, 0x43, 0x39, 0xb8, 0xc3, 0x38, 0x94, 0x43, 0x38, 0x88, 0x03,
  0x3b, 0x94, 0xc3, 0x2f, 0xbc, 0x83, 0x3c, 0xfc, 0x82, 0x3b, 0xd4, 0x03,
  0x3b, 0xb0, 0xc3, 0x8c, 0xcc, 0x21, 0x07, 0x7c, 0x70, 0x03, 0x74, 0x60,
  0x07, 0x37, 0x90, 0x87, 0x72, 0x98, 0x87, 0x77, 0xa8, 0x07, 0x79, 0x18,
  0x87, 0x72, 0x70, 0x83, 0x70, 0xa0, 0x07, 0x7a, 0x90, 0x87, 0x74, 0x10,
  0x87, 0x7a, 0xa0, 0x87, 0x72, 0x00, 0x00, 0x00, 0x00, 0x71, 0x20, 0x00,
  0x00, 0x24, 0x00, 0x00, 0x00, 0x36, 0x30, 0x0d, 0x97, 0xef, 0x3c, 0xfe,
  0xe2, 0x00, 0x83, 0xd8, 0x3c, 0xd4, 0xe4, 0x17, 0xb7, 0x5d, 0x00, 0x5d,
  0xe0, 0x37, 0x0f, 0xb7, 0xe1, 0xec, 0xb2, 0x1c, 0x06, 0x04, 0xce, 0xaa,
  0xd3, 0x70, 0x1b, 0xce, 0x2e, 0xcb, 0xa7, 0xf4, 0x30, 0xbd, 0x0c, 0x04,
  0x06, 0x2b, 0xa0, 0x0d, 0x02, 0x3f, 0x7a, 0x19, 0x4f, 0xaf, 0xcb, 0xcb,
  0x30, 0x20, 0x70, 0x66, 0xfd, 0x91, 0xa8, 0x65, 0x3c, 0xbd, 0x2e, 0x2f,
  0xcb, 0x88, 0x40, 0xeb, 0x8f, 0x64, 0x2f, 0x8f, 0xe9, 0x6f, 0x39, 0xb0,
  0x49, 0x82, 0xcd, 0x80, 0x40, 0x20, 0x30, 0x68, 0x02, 0x04, 0x03, 0x20,
  0x8d, 0x19, 0x48, 0xc3, 0xe5, 0x3b, 0x8f, 0x2f, 0x44, 0x04, 0x30, 0x11,
  0x21, 0xd0, 0x0c, 0x0b, 0x61, 0x05, 0xd0, 0x70, 0xf9, 0xce, 0xe3, 0x4b,
  0x00, 0xf3, 0x2c, 0x84, 0x5f, 0xdc, 0xb6, 0x11, 0x54, 0xc3, 0xe5, 0x3b,
  0x8f, 0x2f, 0x4d, 0x4e, 0x44, 0xa0, 0xd4, 0xf4, 0x50, 0x93, 0x5f, 0xdc,
  0x36, 0x00, 0x00, 0x00, 0x00, 0x61, 0x20, 0x00, 0x00, 0x4d, 0x00, 0x00,
  0x00, 0x13, 0x04, 0x41, 0x2c, 0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
  0x00, 0x04, 0x66, 0x00, 0x4a, 0xae, 0xf0, 0xa8, 0x8e, 0x00, 0xd0, 0x2a,
  0x01, 0x1a, 0x33, 0x00, 0x00, 0x23, 0x06, 0x09, 0x00, 0x82, 0x60, 0x60,
  0x81, 0x41, 0x01, 0x79, 0xde, 0x30, 0x62, 0x90, 0x00, 0x20, 0x08, 0x06,
  0x56, 0x18, 0x18, 0xce, 0xf7, 0x11, 0x23, 0x06, 0x09, 0x00, 0x82, 0x60,
  0xc0, 0x94, 0xc1, 0xd7, 0x81, 0x81, 0x84, 0x8c, 0x18, 0x24, 0x00, 0x08,
  0x82, 0x01, 0x63, 0x06, 0x60, 0xe0, 0x85, 0x41, 0x91, 0x8c, 0x18, 0x24,
  0x00, 0x08, 0x82, 0x01, 0x73, 0x06, 0x61, 0x30, 0x06, 0x62, 0x40, 0x29,
  0x23, 0x06, 0x09, 0x00, 0x82, 0x60, 0xc0, 0xa0, 0x81, 0x18, 0x90, 0xc1,
  0x18, 0x1c, 0xcb, 0x88, 0x41, 0x02, 0x80, 0x20, 0x18, 0x30, 0x69, 0x30,
  0x06, 0x65, 0x40, 0x06, 0x17, 0x33, 0x62, 0x90, 0x00, 0x20, 0x08, 0x06,
  0x8c, 0x1a, 0x90, 0x81, 0x19, 0x94, 0x01, 0xd5, 0x8c, 0x18, 0x3c, 0x00,
  0x08, 0x82, 0xc1, 0x94, 0x06, 0x0c, 0x72, 0x18, 0x45, 0x92, 0x38, 0x8e,
  0x93, 0x8c, 0x26, 0x04, 0xc0, 0x68, 0x82, 0x10, 0x8c, 0x26, 0x0c, 0xc2,
  0x68, 0x02, 0x31, 0x18, 0x91, 0xc8, 0xc7, 0x88, 0x44, 0x3e, 0x46, 0x24,
  0xf2, 0x31, 0x22, 0x91, 0xcf, 0x88, 0x41, 0x02, 0x80, 0x20, 0x18, 0x38,
  0x73, 0x60, 0x06, 0x6f, 0xf0, 0x06, 0x61, 0x40, 0x8c, 0x18, 0x24, 0x00,
  0x08, 0x82, 0x81, 0x33, 0x07, 0x66, 0xf0, 0x06, 0x6f, 0x30, 0x0d, 0x23,
  0x06, 0x09, 0x00, 0x82, 0x60, 0xe0, 0xcc, 0x81, 0x19, 0xbc, 0xc1, 0x1b,
  0x88, 0x81, 0x30, 0x62, 0x90, 0x00, 0x20, 0x08, 0x06, 0xce, 0x1c, 0x98,
  0xc1, 0x1b, 0xbc, 0x81, 0x17, 0x60, 0x38, 0x10, 0x00, 0x0d, 0x00, 0x00,
  0x00, 0x66, 0x32, 0x4d, 0xc4, 0x35, 0x51, 0x11, 0x41, 0xff, 0x13, 0x71,
  0x4d, 0x54, 0x44, 0xfc, 0xf6, 0x60, 0x27, 0x90, 0x04, 0x30, 0xcf, 0x42,
  0x44, 0xf4, 0x2f, 0x01, 0xcc, 0xb3, 0x10, 0x91, 0xcd, 0x0c, 0x3e, 0x42,
  0x1b, 0xcd, 0xe0, 0x23, 0xb5, 0xd5, 0x0c, 0x3e, 0x62, 0x9b, 0xcd, 0xe0,
  0x23, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00
};