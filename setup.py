import os
import platform
import urllib.request
import subprocess
import tarfile
import shutil

def detect_os():
    os_name = platform.system()
    if os_name == "Windows":
        return "Windows"
    elif os_name == "Linux":
        raise SystemExit("Linux is not yet supported")
        return "Linux"
    elif os_name == "Darwin":
        raise SystemExit("macOS is not yet supported")
        return "macOS"
    else:
        raise SystemExit("Unsupported OS")

def download_file(url, output_path):
    print(f"Downloading {url} ...")
    urllib.request.urlretrieve(url, output_path)
    print(f"Downloaded to {output_path}")

def install_vulkan_sdk():
    os_type = detect_os()

    if os_type == "Windows":
        # Download and install Vulkan SDK for Windows
        sdk_url = "https://sdk.lunarg.com/sdk/download/1.3.224.1/windows/VulkanSDK-1.3.224.1-Installer.exe"  # Change as needed
        sdk_installer = "VulkanSDK-1.3.224.1-Installer.exe"
        download_file(sdk_url, sdk_installer)

        # Run the installer
        print("Running the installer...")
        subprocess.run([sdk_installer, "/S"], check=True)  # /S for silent install
        print("Vulkan SDK installed successfully on Windows.")
        return True
    else:
        raise SystemExit("Unsupported OS")
        return False
    
def check_vulkan_sdk():
    vulkan_sdk = os.getenv("VULKAN_SDK")
    if vulkan_sdk:
        print(f"Vulkan SDK found at: {vulkan_sdk}")
        return True
    else:
        print("Vulkan SDK not found!")
        return False
    
def setup_project():
    print("Creating Project...")
    os_type = detect_os()

    if os_type == "Windows":
        subprocess.run(["externals\premake\premake5", "vs2022"], check=True)
    else:
        raise SystemExit("Unsupported OS")



def main():
    has_vk_sdk = check_vulkan_sdk()

    if not has_vk_sdk:
        has_vk_sdk = install_vulkan_sdk()

    if has_vk_sdk:
        setup_project()
    else:
        print("Falied to setup Trace")

    
if __name__ == "__main__":
    main()