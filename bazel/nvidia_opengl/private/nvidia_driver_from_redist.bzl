"""Bazel repository rule for downloading and setting up OpenGL and EGL libraries from NVIDIA.

This rule fetches NVIDIA drivers from the official redistributable source.
It performs the following steps:
1.  Downloads a manifest file for the specified driver version.
2.  Parses the manifest to find the driver package details for the target OS and architecture.
3.  Downloads and extracts the driver archive (tar.xz).
4.  Creates symlinks for essential EGL, OpenGL, GLESv1, and GLESv2 libraries,
    pointing to the versioned library files within the extracted driver package.
5.  Generates a BUILD.bazel file using a template, substituting the driver version.

This allows other Bazel targets to depend on the NVIDIA driver's OpenGL and EGL components.

Why is this not just a http_archive rule with a fixed BUILD file?
- Because the downloaded driver has files that need to be symlinked
  with version suffixes.
- The driver version needs to be substituted into the BUILD file as well as the version suffixes.

Usage example in MODULE.bazel file if used as a repo_rule:
nvidia_driver_from_redist = use_repo_rule("//bazel/nvidia_opengl/private:nvidia_driver_from_redist.bzl", "nvidia_driver_from_redist")
nvidia_driver_from_redist(
    name = "nvidia_driver",
    driver_version = "550.163.01",
)

For module extension usage, see extension.bzl
"""

# List of manifest checksums for supported NVIDIA driver versions.
# This is used to verify the integrity of the downloaded manifest file to protect against dependency attacks.
# The checksums were obtained by downloading the manifest file from https://developer.download.nvidia.com/compute/nvidia-driver/redist/redistrib_VERSION.json
# and looking up the SHA256 checksum using `sha256sum <manifest_file>`.
manifest_checksums = {
    "550.163.01": "9ac168619f1c4427ed867f952baa21a8d365800d0c8f80f329db8661c11b8f8c",
}

def _nvidia_driver_from_redist_impl(repository_ctx):
    driver_version = repository_ctx.attr.driver_version
    target_arch = repository_ctx.attr.arch
    os_name = repository_ctx.attr.os_name
    redist_base_url = repository_ctx.attr._base_download_url
    manifest_filename = "redistrib_%s.json" % driver_version

    if driver_version not in manifest_checksums:
        fail("NVIDIA Rule: Unsupported driver version: %s" % driver_version)

    repository_ctx.download(
        url = redist_base_url + manifest_filename,
        output = manifest_filename,
        sha256 = manifest_checksums[driver_version],
    )

    manifest_content = repository_ctx.read(manifest_filename)
    parsed_manifest = json.decode(manifest_content)
    driver_package_details = parsed_manifest["nvidia_driver"]["%s-%s" % (os_name, target_arch)]
    archive_relative_path = driver_package_details.get("relative_path")
    archive_sha256 = driver_package_details.get("sha256")

    if not archive_relative_path or not archive_sha256:
        fail("NVIDIA Rule: Missing 'relative_path' or 'sha256' for driver archive in manifest.")

    archive_full_url = redist_base_url + archive_relative_path
    extraction_output_dir = "nvidia_driver"

    strip_prefix_val = "nvidia_driver-%s-%s-%s-archive" % (os_name, target_arch.split("-")[-1], driver_version)

    repository_ctx.download_and_extract(
        url = archive_full_url,
        output = extraction_output_dir,
        sha256 = archive_sha256,
        type = "tar.xz",
        stripPrefix = strip_prefix_val,
    )

    # Download and extract X11 libraries, required by EGL and OpenGL.
    # These are depended upon in BUILD.nvidia_driver.tpl.
    # TODO(b/430264123): Build from source
    sysroot_output_dir = "sysroot_x_runtime"
    repository_ctx.download_and_extract(
        url = "https://storage.googleapis.com/chrome-linux-sysroot/toolchain/4f611ec025be98214164d4bf9fbe8843f58533f7/debian_bullseye_amd64_sysroot.tar.xz",
        output = sysroot_output_dir,
        sha256 = "5df5be9357b425cdd70d92d4697d07e7d55d7a923f037c22dc80a78e85842d2c",
        type = "tar.xz",
    )

    # Create symlink with version names suffixes as expected by our tests
    libs_to_symlink = {
        # --- EGL Stack ---
        "libEGL.so.1": "libEGL.so.1.1.0",
        # NVIDIA's specific EGL implementation for GLVND (needs .so.0 name for dlopen)
        "libEGL_nvidia.so.0": "libEGL_nvidia.so." + driver_version,

        # --- OpenGL Stack (via EGL, using GLVND) ---
        "libGL.so.1": "libGL.so.1.7.0",
        "libGLX_nvidia.so.0": "libGLX_nvidia.so." + driver_version,
        # GLESv1
        "libGLESv1_CM.so.1": "libGLESv1_CM.so.1.2.0",  # GLlsVND GLESv1 entry
        # GLESv2
        "libGLESv2.so.2": "libGLESv2.so.2.1.0",  # GLVND GLESv2 entry
    }

    for symlink_name, original_filename in libs_to_symlink.items():
        link_path_in_repo = extraction_output_dir + "/lib/" + symlink_name
        target_for_symlink_attr = extraction_output_dir + "/lib/" + original_filename
        repository_ctx.symlink(
            target_for_symlink_attr,
            link_path_in_repo,
        )

    substitutions = {
        "%{driver-version}": driver_version,
    }

    repository_ctx.template(
        "BUILD.bazel",
        repository_ctx.attr._build_file_template,
        substitutions,
        False,
    )

nvidia_driver_from_redist = repository_rule(
    implementation = _nvidia_driver_from_redist_impl,
    attrs = {
        "driver_version": attr.string(mandatory = True, doc = "NVIDIA driver version, e.g., '550.163.01'"),
        "arch": attr.string(default = "x86_64", doc = "Architecture (e.g., x86_64)."),
        "os_name": attr.string(default = "linux", doc = "OS name, e.g., 'linux'."),
        "_base_download_url": attr.string(default = "https://developer.download.nvidia.com/compute/nvidia-driver/redist/"),
        "_build_file_template": attr.label(
            default = Label("//bazel/nvidia_opengl/private:BUILD.tpl"),
            allow_single_file = True,
        ),
    },
)
