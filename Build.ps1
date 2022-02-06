# set-psdebug -Off
$build_deps = $args[0]
$flags  = "/std:c++17 /W2 /GR- /Zi /EHsc /utf-8"
$deps   = "..\Source\ThirdParty"
if ($build_deps) {
    echo "Building dependencies"
}

new-item -Force -Name "Build" -ItemType "Directory"
pushd Build
    cl $flags /c $deps\imgui.cpp 
popd