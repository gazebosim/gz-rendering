"""
Bazel module extension for NVIDIA OpenGL and EGL libraries.

For usage in MODULE.bazel:
nvidia_driver_ext = use_extension(
    "//bazel/nvidia_opengl:extension.bzl",
    "nvidia_driver_ext",
)
nvidia_driver_ext.nvidia_driver(
    name = "nvidia_driver",
    driver_version = "550.163.01", # change to the desired driver version
)
use_repo(nvidia_driver_ext, "nvidia_driver")
"""
load("//bazel/nvidia_opengl/private:nvidia_driver_from_redist.bzl", "nvidia_driver_from_redist")

def _nvidia_driver_ext_impl(module_ctx):
    """Implementation of the nvidia_driver_ext module extension."""
    for module in module_ctx.modules:
        for repo_tag in module.tags.nvidia_driver:
            nvidia_driver_from_redist(
                name = repo_tag.name,
                driver_version = repo_tag.driver_version,
                arch = repo_tag.arch,
                os_name = repo_tag.os_name,
            )

    module_ctx.extension_metadata(reproducible = True)

_nvidia_driver_tags = tag_class(
    attrs = {
        "name": attr.string(
            mandatory = True,
            doc = "Name of the NVIDIA driver repository tag.",
        ),
        "driver_version": attr.string(
            mandatory = True,
            doc = "NVIDIA driver version, e.g., '550.163.01'",
        ),
        "arch": attr.string(default = "x86_64", doc = "Architecture (e.g., x86_64)."),
        "os_name": attr.string(default = "linux", doc = "OS name, e.g., 'linux'."),
    },
)

# Create the public module extension
nvidia_driver_ext = module_extension(
    implementation = _nvidia_driver_ext_impl,
    tag_classes = {"nvidia_driver": _nvidia_driver_tags},
)