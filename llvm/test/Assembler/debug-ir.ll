; RUN: llvm-as -d -debug-ir %s 2>&1 | FileCheck %s


; CHECK: define dso_local i32 @foo(i8*) !dbg ![[FOOMD:[0-9]+]]
; CHECK: DISubprogram name: "bar" [[FOOMD]]
define dso_local i32 @foo(i8*) #0 {
  %2 = alloca i8*, align 8
  %3 = alloca i8, align 1
  store i8* %0, i8** %2, align 8
  %4 = load i8*, i8** %2, align 8
  %5 = load i8, i8* %4, align 1
  store i8 %5, i8* %3, align 1
  %6 = load i8*, i8** %2, align 8
  %7 = ptrtoint i8* %6 to i32
  ret i32 %7
}

define dso_local i8* @bar(i32*, double, i32) #0 {
  %4 = alloca i32*, align 8
  %5 = alloca double, align 8
  %6 = alloca i32, align 4
  store i32* %0, i32** %4, align 8
  store double %1, double* %5, align 8
  store i32 %2, i32* %6, align 4
  %7 = load i32*, i32** %4, align 8
  %8 = load i32, i32* %6, align 4
  %9 = zext i32 %8 to i64
  %10 = getelementptr inbounds i32, i32* %7, i64 %9
  %11 = bitcast i32* %10 to i8*
  ret i8* %11
}

define dso_local void @baz() #0 {
  ret void
}

define dso_local i32 @main(i32, i8**) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i8**, align 8
  %6 = alloca i8, align 1
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  store i32 0, i32* %3, align 4
  store i32 %0, i32* %4, align 4
  store i8** %1, i8*** %5, align 8
  store i8 97, i8* %6, align 1
  %9 = call i32 @foo(i8* %6)
  store i32 %9, i32* %7, align 4
  store i32 2, i32* %8, align 4
  %10 = load i32, i32* %7, align 4
  %11 = call i8* @bar(i32* %8, double 2.000000e+00, i32 %10)
  ret i32 0
}
