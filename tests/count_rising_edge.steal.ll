; ModuleID = 'tests/count_rising_edge.steal.bc'
source_filename = "tests/count_rising_edge.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.List = type { i8, ptr }

@__const.main.fourth = private unnamed_addr constant %struct.List { i8 1, ptr null }, align 8
@.str = private unnamed_addr constant [18 x i8] c"rising_edges == 1\00", align 1
@.str.1 = private unnamed_addr constant [26 x i8] c"tests/count_rising_edge.c\00", align 1
@__PRETTY_FUNCTION__.main = private unnamed_addr constant [23 x i8] c"int main(int, char **)\00", align 1
@.str.2 = private unnamed_addr constant [31 x i8] c"Rising edges in the list: %ld\0A\00", align 1

; Function Attrs: nounwind sspstrong uwtable
define dso_local i32 @main(i32 noundef %0, ptr nocapture noundef readnone %1) local_unnamed_addr #0 {
  %3 = alloca %struct.List, align 8
  %4 = alloca %struct.List, align 8
  %5 = alloca %struct.List, align 8
  %6 = alloca %struct.List, align 8
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %3) #7
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %3, ptr noundef nonnull align 8 dereferenceable(16) @__const.main.fourth, i64 16, i1 false)
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %4) #7
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %4, i8 0, i64 16, i1 false)
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %5) #7
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %5, ptr noundef nonnull align 8 dereferenceable(16) @__const.main.fourth, i64 16, i1 false)
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %6) #7
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %6, ptr noundef nonnull align 8 dereferenceable(16) @__const.main.fourth, i64 16, i1 false)
  %7 = getelementptr inbounds %struct.List, ptr %3, i64 0, i32 1
  store ptr %4, ptr %7, align 8, !tbaa !5
  %8 = getelementptr inbounds %struct.List, ptr %4, i64 0, i32 1
  store ptr %5, ptr %8, align 8, !tbaa !5
  %9 = getelementptr inbounds %struct.List, ptr %5, i64 0, i32 1
  store ptr %6, ptr %9, align 8, !tbaa !5
  %10 = call fastcc i64 @count_rising_edge(ptr noundef nonnull %3, i64 noundef 0, i1 noundef zeroext true)
  %11 = icmp eq i64 %10, 1
  br i1 %11, label %13, label %12

12:                                               ; preds = %2
  call void @__assert_fail(ptr noundef nonnull @.str, ptr noundef nonnull @.str.1, i32 noundef 32, ptr noundef nonnull @__PRETTY_FUNCTION__.main) #8
  unreachable

13:                                               ; preds = %2
  %14 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.2, i64 noundef 1)
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %6) #7
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %5) #7
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %4) #7
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %3) #7
  ret i32 0
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #3

; Function Attrs: nofree noinline nosync nounwind sspstrong memory(argmem: read) uwtable
define internal fastcc i64 @count_rising_edge(ptr noundef readonly %0, i64 noundef %1, i1 noundef zeroext %2) unnamed_addr #4 {
  br label %4

4:                                                ; preds = %13, %3
  %5 = phi ptr [ %0, %3 ], [ %17, %13 ]
  %6 = phi i64 [ %1, %3 ], [ %15, %13 ]
  %7 = phi i1 [ %2, %3 ], [ %19, %13 ]
  %8 = icmp eq ptr %5, null
  br i1 %8, label %20, label %9

9:                                                ; preds = %4
  br i1 %7, label %13, label %10

10:                                               ; preds = %9
  %11 = load i8, ptr %5, align 8, !tbaa !11, !range !12, !noundef !13
  %12 = zext i8 %11 to i64
  br label %13

13:                                               ; preds = %10, %9
  %14 = phi i64 [ 0, %9 ], [ %12, %10 ]
  %15 = add i64 %14, %6
  %16 = getelementptr inbounds %struct.List, ptr %5, i64 0, i32 1
  %17 = load ptr, ptr %16, align 8, !tbaa !5
  %18 = load i8, ptr %5, align 8, !tbaa !11, !range !12, !noundef !13
  %19 = icmp ne i8 %18, 0
  br label %4

20:                                               ; preds = %4
  ret i64 %6
}

; Function Attrs: noreturn nounwind
declare void @__assert_fail(ptr noundef, ptr noundef, i32 noundef, ptr noundef) local_unnamed_addr #5

; Function Attrs: nofree nounwind
declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #6

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind sspstrong uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #3 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #4 = { nofree noinline nosync nounwind sspstrong memory(argmem: read) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { noreturn nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { nounwind }
attributes #8 = { noreturn nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 17.0.6"}
!5 = !{!6, !10, i64 8}
!6 = !{!"List", !7, i64 0, !10, i64 8}
!7 = !{!"_Bool", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!"any pointer", !8, i64 0}
!11 = !{!6, !7, i64 0}
!12 = !{i8 0, i8 2}
!13 = !{}
