#include <iostream>
#include "MyApplication.h"
#include <DirectXMath.h>
#include <shaderc/shaderc.hpp>
#include <fstream>
#include <sstream>

using namespace std;

void PrintVec(DirectX::XMMATRIX mat,const char* tag = nullptr)
{
    if (tag)
        printf("%s\n", tag);

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            printf("%f  ", mat.r[i].m128_f32[j]);
        }
        printf("\n");
    }
}

void PrintVec(DirectX::XMVECTOR vec, const char* tag = nullptr)
{
    if (tag)
        printf("%s\n", tag);

    for (int j = 0; j < 4; ++j)
    {
        printf("%f  ", vec.m128_f32[j]);
    }
    printf("\n");
}

void ShadercTest()
{
    string file = "shaders/triangle.vert";
    //file = "123.txt";

    ifstream in(file, ios::binary);
    if (!in.is_open()) return;
    in.seekg(0, ios::end);
    int size = in.tellg();
    in.seekg(0, ios::beg);
    char* source = new char[size];
    in.read(source, size);
    in.close();

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    auto result = compiler.CompileGlslToSpv(source, size, shaderc_shader_kind::shaderc_vertex_shader, "Triangle", "main", options);
    delete[] source;
    source = nullptr;

    auto status = result.GetCompilationStatus();
    if (status != shaderc_compilation_status::shaderc_compilation_status_success)
    {
        cout << "Compile Shader Fail !!" << endl;

        cout << result.GetErrorMessage() << endl;
    }

}

int WINAPI WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    //¿ªÆô¿ØÖÆÌ¨
    AllocConsole();
    freopen("CONOUT$", "w+t", stdout);
    
    uint32_t Width = 1280;
    uint32_t Height = 800;

    ShadercTest();

    //DirectX::XMMATRIX mat(
    //    1.0f, 0.0f, 0.0f, 0.0f,
    //    0.0f, 1.0f, 0.0f, 0.0f,
    //    0.0f, 0.0f, 1.0f, 0.0f,
    //    0.0f, 0.0f, 0.0f, 1.0f
    //);

    //DirectX::XMVECTOR vec{ 0.0f,0.0f,0.0f,0.0f };
    //auto rom = DirectX::XMMatrixRotationZ(3.1415926f/2.0f);
    //PrintVec(rom,"Matrix Name : mat ");
    //PrintVec(vec, "Vector Name : vec");
    
    // create new DX sample
    return RunFramework(hInstance, lpCmdLine, nCmdShow, Width, Height, new MyApplication(Width,Height));
}