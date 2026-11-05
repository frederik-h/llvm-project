; RUN: llc -mtriple=amdgpu9.50 -O1 -stop-after=greedy %s -o - | FileCheck %s

; CHECK-LABEL: $exec = S_OR_B64 $exec
; CHECK-NEXT: ATOMIC_FENCE 5, 1

; SI_END_CF should not be collapsed, i.e. the EXEC restore must remain
; before the ATOMIC_FENCE.

define amdgpu_kernel void @foo(i1 %cmp.1) {
entry:
  br label %for.body.1

for.body.1:
  br i1 %cmp.1, label %if.then.1, label %if.end.1

common.ret:
  ret void

if.then.1:
  br label %if.end.1

if.end.1:
  %phi = phi <1 x i32> [ zeroinitializer, %if.then.1 ], [ splat (i32 1), %for.body.1 ]
  %extract = extractelement <1 x i32> %phi, i64 0
  %cmp2 = icmp eq i32 %extract, 0
  br i1 %cmp2, label %if.then.2, label %for.header.468.backedge

for.header.468.backedge:
  br i1 %cmp.1, label %for.body.1, label %common.ret

if.then.2:
  br i1 %cmp.1, label %for.exit, label %for.body.2

for.body.2:
  %asm1 = tail call <4 x i32> asm sideeffect "", "=&v,0,~{memory}"(<4 x i32> zeroinitializer)
  ret void

for.exit:
  %asm2 = tail call i32 asm sideeffect "", "=v,0"(i32 0)
  %cmp3 = icmp eq i32 %asm2, 0
  br i1 %cmp3, label %if.end.2, label %if.end.3

if.end.2:
  tail call void asm sideeffect "global_atomic_add $0, $1, off sc1", "v,v,~{memory}"(i64 0, i32 0)
  br label %if.end.3

if.end.3:
  fence release
  ret void
}
