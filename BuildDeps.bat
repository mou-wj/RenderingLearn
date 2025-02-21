
mkdir deps
cd deps
if not exist "SPIRV-Cross" (
	git clone --branch main https://github.com/KhronosGroup/SPIRV-Cross.git
)
if not exist "./install/SPIRV-Cross" (
	cmake -S ./SPIRV-Cross -B ./build -DCMAKE_INSTALL_PREFIX=./install/SPIRV-Cross/Debug
	cmake --build ./build --config Debug --target install
	rmdir /s /q build
	cmake -S ./SPIRV-Cross -B ./build -DCMAKE_INSTALL_PREFIX=./install/SPIRV-Cross/Release
	cmake --build ./build --config Release --target install
	rmdir /s /q build
)


if not exist "SPIRV-Tools" (
	git clone --branch main https://github.com/KhronosGroup/SPIRV-Tools.git
	git clone --branch main https://github.com/KhronosGroup/SPIRV-Headers.git ./SPIRV-Tools/external/spirv-headers
)

if not exist "./install/SPIRV-Tools" (
	cmake -S ./SPIRV-Tools -B ./build -DCMAKE_INSTALL_PREFIX=./install/SPIRV-Tools/Debug
	cmake --build ./build --config Debug --target install
	rmdir /s /q build
	cmake -S ./SPIRV-Tools -B ./build -DCMAKE_INSTALL_PREFIX=./install/SPIRV-Tools/Release
	cmake --build ./build --config Release --target install
	rmdir /s /q build
)



if not exist "glslang" (
	git clone --branch main https://github.com/KhronosGroup/glslang.git
)

if not exist "./install/glslang" (
	cmake -S ./glslang -B ./build -DCMAKE_INSTALL_PREFIX=./install/glslang/Debug -DENABLE_OPT=OFF
	cmake --build ./build --config Debug --target install
	rmdir /s /q build
	cmake -S ./glslang -B ./build -DCMAKE_INSTALL_PREFIX=./install/glslang/Release -DENABLE_OPT=OFF
	cmake --build ./build --config Release --target install
	rmdir /s /q build
)


if not exist "glfw" (
	git clone --branch master https://github.com/glfw/glfw.git
)

if not exist "./install/glfw" (
	cmake -S ./glfw -B ./build -DCMAKE_INSTALL_PREFIX=./install/glfw/Debug
	cmake --build ./build --config Debug --target install
	rmdir /s /q build
	cmake -S ./glfw -B ./build -DCMAKE_INSTALL_PREFIX=./install/glfw/Release
	cmake --build ./build --config Release --target install
	rmdir /s /q build
)


if not exist "tinygltf" (
	git clone --branch release https://github.com/syoyo/tinygltf.git
)

if not exist "./install/tinygltf" (
	cmake -S ./tinygltf -B ./build -DCMAKE_INSTALL_PREFIX=./install/tinygltf/Debug
	cmake --build ./build --config Debug --target install
	rmdir /s /q build
	cmake -S ./tinygltf -B ./build -DCMAKE_INSTALL_PREFIX=./install/tinygltf/Release
	cmake --build ./build --config Release --target install
	rmdir /s /q build
)
pause