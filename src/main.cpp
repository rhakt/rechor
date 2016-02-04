// rechor project
// main.cpp

#include <iostream>
#include "main.hpp"

#define QUOTE(str) "\"" << str << "\""


int main(int argc, char* argv[]) {

#if _DEBUG && _MSC_VER
    // check memory leak (only VS)
    ::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    std::atexit([](){
        std::system("pause");
    });
#endif

    std::cout << "[RECHOR]" << std::endl;

    const char* fi = "model/unitychan.fbx";
    const char* fi2 = "model/unitychan_WAIT04.fbx";
    const char* fi3 = "model/unitychan_WIN00.fbx";
    const char* fo = "model/unitychan.rchr";
    
    rechor::FBXImporter importer;
    rechor::Exporter exporter;
    rechor::Importer reimporter;
    rechor::Scene scene, scene2;
    
    using OPTION = rechor::FBXImporter::OPTION;
    
    if(!importer.load(fi, OPTION::LOAD_MESH | OPTION::LOAD_BONEWEIGHT)){
        std::cout << "fail to load " << QUOTE(fi) << std::endl;
        return -1;
    }
    if (!importer.load(fi2, OPTION::LOAD_ANIM)){
        std::cout << "fail to load " << QUOTE(fi2) << std::endl;
        return -1;
    }
    if(!importer.load(fi3, scene, OPTION::LOAD_ANIM)){
        std::cout << "fail to load " << QUOTE(fi3) << std::endl;
        return -1;
    }
 
    if (!exporter.save(fo, scene)){
        std::cout << "fail to save " << QUOTE(fo) << std::endl;
        return -1;
    }
    
    /*if (!reimporter.load(fo, scene2)){
        std::cout << "fail to load " << QUOTE(fo) << std::endl;
        return -1;
    }*/
    

    return 0;
}