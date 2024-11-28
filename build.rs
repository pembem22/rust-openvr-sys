#[cfg(feature = "buildtime_bindgen")]
extern crate bindgen;
extern crate cmake;

use std::env;

fn main() {
    let mut config = cmake::Config::new("openvr");
    let target_os = env::var("CARGO_CFG_TARGET_OS").unwrap();
    let target_arch = std::env::var("CARGO_CFG_TARGET_ARCH").unwrap();
    let target_pointer_width = env::var("CARGO_CFG_TARGET_POINTER_WIDTH").unwrap();
    let target_env = std::env::var("CARGO_CFG_TARGET_ENV").unwrap();

    if target_os == "macos" {
        config.define("BUILD_UNIVERSAL", "OFF");
    } else if target_os == "windows" {
        // Work around broken cmake build.
        config.cxxflag("/DWIN32");
        // config.cxxflag("/EHsc");

        // // Workaround: Microsoft extensions aren't enabled by default for the `gnu` toolchain
        // // on Windows. We need to enable it manually, or MSVC headers will fail to parse. We
        // // also need to manually define architecture macros, or the build will fail with an
        // // "Unsupported architecture" error.
        // //
        // // This does not happen when the `msvc` toolchain is used.
        // if target_env == "gnu" {
        //     let arch_macro = match target_arch.as_str() {
        //         "x86" => "_M_IX86",
        //         "x86_64" => "_M_X64",
        //         "arm" => "_M_ARM",
        //         "aarch64" => "_M_ARM64",
        //         _ => panic!("architecture {target_arch} not supported on Windows"),
        //     };

        //     config.build_arg("-fms-extensions");
        //     config.build_arg("-fmsc-version=1300");
        //     config.build_arg(format!("-D{arch_macro}=100"));
        // }
    }

    let dst = config.build();
    println!("cargo:rustc-link-search=native={}/lib", dst.display());

    if target_os == "windows" && target_pointer_width == "64" {
        println!("cargo:rustc-link-lib=static=openvr_api64");
    } else {
        println!("cargo:rustc-link-lib=static=openvr_api");
    }

    if target_os == "linux" {
        println!("cargo:rustc-link-lib=stdc++");
    } else if target_os == "macos" {
        println!("cargo:rustc-link-lib=c++");
    } else if target_os == "windows" {
        println!("cargo:rustc-link-lib=shell32");
    }

    // Generate bindings at build time.
    #[cfg(feature = "buildtime_bindgen")]
    bindgen::builder()
        .header("wrapper.hpp")
        .constified_enum(".*")
        .prepend_enum_name(false)
        .vtable_generation(true)
        .generate()
        .expect("could not generate bindings")
        .write_to_file("bindings.rs")
        .expect("could not write bindings.rs");
}
