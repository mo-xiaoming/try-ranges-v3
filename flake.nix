{
  description = "cpp project with nix";

  outputs = { self, nixpkgs }: with nixpkgs.legacyPackages.x86_64-linux; {
    drv-attrs = {
      pname = "cpp-nix-template";
      version = "0.0.1";

      src = self;

      nativeBuildInputs = [ cmake ninja gdb valgrind ];

      buildInputs = [ spdlog doctest boost175 ranges-v3 ];

      doCheck = true;
      checkPhase = "ctest --output-on-failure";

      UBSAN_OPTIONS="print_stacktrace=1";
      ASAN_OPTIONS="detect_leaks=1:strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1:use_odr_indicator=1";
    };

    packages.x86_64-linux = {
      gcc-pkg = gcc11Stdenv.mkDerivation self.drv-attrs;
      clang-pkg = (llvmPackages_13.stdenv.mkDerivation self.drv-attrs).overrideAttrs (oa: {
        CPATH = lib.makeSearchPathOutput "dev" "include" oa.buildInputs;
      });
    };

    defaultPackage.x86_64-linux = self.packages.x86_64-linux.gcc-pkg;
  };
}
