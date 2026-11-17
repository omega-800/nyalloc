{
  description = "c development environment";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      inherit (nixpkgs.lib)
        genAttrs
        platforms
        fileset
        ;
      systems = platforms.unix;
      eachSystem =
        f:
        genAttrs systems (
          system:
          f (
            import nixpkgs {
              inherit system;
              config = { };
              overlays = [ ];
            }
          )
        );
      fs = fileset;
      root = ./.;
      src = fs.toSource {
        inherit root;
        fileset = fs.intersection (fs.gitTracked root) (
          fs.fileFilter (f: (f.name == "Makefile") || (f.hasExt "c") || (f.hasExt "h")) root
        );
      };
      pname = "nyalloc";
    in
    {
      asdf = src;
      devShells = eachSystem (pkgs: {
        default = pkgs.mkShellNoCC {
          packages = with pkgs; [
            tinycc
            gnumake
            gcc
          ];
        };
      });

      packages = eachSystem (
        pkgs:
        let
          inherit (pkgs) stdenv;
          mkTarget =
            target:
            stdenv.mkDerivation {
              inherit pname;
              version = "0.0.1${if target == "release" then "" else "-" + target}";
              inherit src;

              buildPhase = ''
                mkdir -p "$out/bin"
              '';
              installPhase = ''
                OUT="${pname}" TARGETDIR="$out/bin" make ${target}
              '';
            };
        in
        {
          default = mkTarget "release";
          debug = mkTarget "debug";
        }
      );

      apps = eachSystem (pkgs: {
        default = {
          type = "app";
          program = "${self.packages.${pkgs.stdenv.hostPlatform.system}.default}/bin/${pname}";
        };
      });
    };
}
