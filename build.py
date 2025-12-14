from os import system
import platform
import subprocess


def pre_build_checks():
	print("Performing pre-build checks...")
	
	required_tools = ["cmake", "conan", "clang", "ninja"]
	
	missing_tools = []
	
	for tool in required_tools:
		try:
			result = subprocess.run(
				["where", tool] if platform.system() == "Windows" else ["which", tool],
				capture_output=True,
				text=True,
				timeout=5
			)
			if result.returncode != 0:
				missing_tools.append(tool)
		except Exception as e:
			print(f"Error checking for {tool}: {e}")
			missing_tools.append(tool)
	
	# Check for Visual Studio on Windows
	if platform.system() == "Windows":
		vs_found = False
		vs_base_paths = [
			r"C:\Program Files\Microsoft Visual Studio",
			r"C:\Program Files (x86)\Microsoft Visual Studio",
		]
		
		for base_path in vs_base_paths:
			try:
				if subprocess.run(["cmd", "/c", f"if exist \"{base_path}\" (exit 0) else (exit 1)"], capture_output=True, timeout=5).returncode == 0:
					vs_found = True
					break
			except Exception:
				pass
		
		if not vs_found:
			print("Warning: Visual Studio is not detected.")
			print("Please download and install Visual C++ Build Tools or Visual Studio Community Edition.")
			print("You can download from: https://visualstudio.microsoft.com/downloads/")
	
	if missing_tools:
		print(f"Error: The following required tools are not installed: {', '.join(missing_tools)}")
		print("Please install the missing tools and try again.")
		exit(1)
	
	print("All required tools are installed. Proceeding with build...")


def build_project():
	print("Building the project...")
	pre_build_checks()
	
	system("conan profile detect")
	
	if system('conan install . -of .install -b missing -o "&:build_app=True" -s "&:compiler=clang" -s "&:compiler.version=20" -s compiler.cppstd=20 -s build_type=Release -c tools.cmake.cmaketoolchain:generator=Ninja') != 0:
		print("Failed to install conan dependencies. Aborting build.")
		exit(1)
	
	if system("cmake --preset release") != 0:
		print("Failed to configure CMake. Aborting build.")
		exit(1)
	
	if system("cmake --build .build/release") == 0:
		print("Build completed successfully.")
	else:
		print("Build failed. Aborting.")
		exit(1)

if __name__ == "__main__":
	build_project()

