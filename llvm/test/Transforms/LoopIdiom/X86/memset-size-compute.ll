; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt -loop-idiom -S %s | FileCheck %s

target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx11.0.0"

; Make sure the number of bytes is computed correctly in the presence of zero
; extensions and preserved add flags.
define void @test(i64* %ptr) {
; CHECK-LABEL: @test(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    br label [[FOR_BODY_PREHEADER:%.*]]
; CHECK:       dead:
; CHECK-NEXT:    br label [[FOR_BODY_PREHEADER]]
; CHECK:       for.body.preheader:
; CHECK-NEXT:    [[LIM_0:%.*]] = phi i32 [ 65, [[ENTRY:%.*]] ], [ 1, [[DEAD:%.*]] ]
; CHECK-NEXT:    [[SCEVGEP:%.*]] = getelementptr i64, i64* [[PTR:%.*]], i64 1
; CHECK-NEXT:    [[SCEVGEP1:%.*]] = bitcast i64* [[SCEVGEP]] to i8*
; CHECK-NEXT:    [[TMP0:%.*]] = icmp ugt i32 [[LIM_0]], 2
; CHECK-NEXT:    [[UMAX:%.*]] = select i1 [[TMP0]], i32 [[LIM_0]], i32 2
; CHECK-NEXT:    [[TMP1:%.*]] = add nsw i32 [[UMAX]], -1
; CHECK-NEXT:    [[TMP2:%.*]] = zext i32 [[TMP1]] to i64
; CHECK-NEXT:    [[TMP3:%.*]] = shl nuw nsw i64 [[TMP2]], 3
; CHECK-NEXT:    call void @llvm.memset.p0i8.i64(i8* align 8 [[SCEVGEP1]], i8 0, i64 [[TMP3]], i1 false)
; CHECK-NEXT:    br label [[FOR_BODY:%.*]]
; CHECK:       for.body:
; CHECK-NEXT:    [[IV:%.*]] = phi i32 [ [[IV_NEXT:%.*]], [[FOR_BODY]] ], [ 1, [[FOR_BODY_PREHEADER]] ]
; CHECK-NEXT:    [[IV_EXT:%.*]] = zext i32 [[IV]] to i64
; CHECK-NEXT:    [[GEP:%.*]] = getelementptr inbounds i64, i64* [[PTR]], i64 [[IV_EXT]]
; CHECK-NEXT:    [[IV_NEXT]] = add nuw nsw i32 [[IV]], 1
; CHECK-NEXT:    [[CMP64:%.*]] = icmp ult i32 [[IV_NEXT]], [[LIM_0]]
; CHECK-NEXT:    br i1 [[CMP64]], label [[FOR_BODY]], label [[EXIT:%.*]]
; CHECK:       exit:
; CHECK-NEXT:    ret void
;
entry:
  br label %for.body.preheader

dead:
  br label %for.body.preheader

for.body.preheader:
  %lim.0 = phi i32 [ 65, %entry ], [ 1, %dead ]
  br label %for.body

for.body:
  %iv = phi i32 [ %iv.next, %for.body ], [ 1, %for.body.preheader ]
  %iv.ext = zext i32 %iv to i64
  %gep = getelementptr inbounds i64, i64* %ptr, i64 %iv.ext
  store i64 0, i64* %gep, align 8
  %iv.next = add nuw nsw i32 %iv, 1
  %cmp64 = icmp ult i32 %iv.next, %lim.0
  br i1 %cmp64, label %for.body, label %exit

exit:
  ret void
}
