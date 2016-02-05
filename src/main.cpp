// rechor project
// main.cpp

#include <iostream>
#include "main.hpp"

auto main(int argc, char* argv[])-> int {

#if _DEBUG && _MSC_VER
    // check memory leak (only VS)
    ::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    std::atexit([](){
        std::system("pause");
    });
#endif

    using logger = rechor::logger;
    logger::setLevel(rechor::LOGLEVEL::DEBUG);

    logger::info("[RECHOR]");

    const char* fi = "model/unitychan.fbx";
    const char* fi2 = "model/unitychan_WAIT04.fbx";
    const char* fi3 = "model/unitychan_WIN00.fbx";
    const char* fo = "model/unitychan.rkr";
    const char* fo2 = "model/unitychan2.rkr";
    
    rechor::FBXImporter importer;
    rechor::Exporter exporter, reexporter;
    rechor::Importer reimporter;
    rechor::Scene scene, scene2;
    
    using OPTION = rechor::FBXImporter::OPTION;
    
    if(!importer.load(fi, OPTION::LOAD_MESH | OPTION::LOAD_BONEWEIGHT)) {
        logger::error("fail to load ", '"', fi, '"');
        return -1;
    }
    if(!importer.load(fi2, OPTION::LOAD_ANIM)) {
        logger::error("fail to load ", '"', fi2, '"');
        return -1;
    }
    if(!importer.load(fi3, scene, OPTION::LOAD_ANIM)) {
        logger::error("fail to load ", '"', fi3, '"');
        return -1;
    }
 
    if(!exporter.save(fo, scene)) {
        logger::error("fail to save ", '"', fo, '"');
        return -1;
    }
    
    if(!reimporter.load(fo, scene2)) {
        logger::error("fail to load ", '"', fo, '"');
        return -1;
    }
    
    if(!reexporter.save(fo2, scene2)) {
        logger::error("fail to save ", '"', fo2, '"');
        return -1;
    }
    logger::info("finish!");
    
}