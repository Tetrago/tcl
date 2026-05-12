{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    systems.url = "github:nix-systems/default";
  };

  outputs =
    {
      nixpkgs,
      systems,
      self,
      ...
    }:
    let
      eachSystem = nixpkgs.lib.genAttrs (import systems);
    in
    {
      devShells = eachSystem (
        system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
        in
        {
          default = (pkgs.mkShell.override { stdenv = pkgs.clangStdenv; }) {
            nativeBuildInputs = with pkgs; [
              cmake
              gdb
              libllvm
              ninja
            ];
          };
        }
      );

      packages = eachSystem (
        system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
        in
        {
          default = pkgs.stdenv.mkDerivation rec {
            pname = "tcl";
            version = self.shortRev or "dirty";

            src = ./.;

            nativeBuildInputs = [
              pkgs.cmake
            ];

            cmakeFlags = [
              "-DTCL_BUILD_TESTS=OFF"
              "-DTCL_VERSION=${version}"
            ];
          };
        }
      );
    };
}
