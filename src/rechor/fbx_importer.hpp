// rechor project
// fbx_importer.hpp

#ifndef _RHACT_RECHOR_FBX_IMPORTER_HPP_
#define _RHACT_RECHOR_FBX_IMPORTER_HPP_

#include <vector>
#include <array>
#include <tuple>
#include <initializer_list>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <algorithm>

#include <fbxsdk.h>

#include "rechor.hpp"

namespace rhakt {
namespace rechor {

    typedef std::array<float, 3> vertex_t;
    typedef std::array<float, 3> normal_t;
    typedef std::array<float, 4> color_t;
    typedef std::array<float, 2> uv_t;
    typedef std::array<uint,  4> bindex_t;
    typedef std::array<float, 4> bweight_t;
    typedef std::tuple<vertex_t, normal_t, color_t, uv_t, bindex_t, bweight_t> element_t;
    
    struct AnimFrameRaw {
        std::vector<std::vector<float>> meshMatrices;
        std::vector<std::vector<float>> boneMatrices;
    };

    struct AnimRaw {
        std::string name;
        int start;
        int end;
        std::vector<AnimFrameRaw> meshes;
    };

    
    struct MeshRaw {
        std::string nodeName;
        std::vector<uint> indices;
        std::vector<vertex_t> vertices;
        std::vector<normal_t> normals;
        std::vector<color_t> colors;
        std::vector<uv_t> uvs;
        std::string texture;
        std::vector<bindex_t> boneIndices;
        std::vector<bweight_t> boneWeights;
        /*-- temp --*/
        std::vector<std::string> boneNodeNames;
        std::vector<FbxMatrix> invBoneBasePoseMatrices;
        FbxAMatrix invMeshBasePoseMatrix;
    };

    struct SceneRaw {
        std::vector<MeshRaw> meshes;
        std::vector<AnimRaw> animes;
    };
    

    class FBXImporter : private util::Noncopyable {
    public:
        
        typedef int FBX_IMPORTER_OPTION;
        struct OPTION {
            static const FBX_IMPORTER_OPTION LOAD_POLYGON   = 0x01;
            static const FBX_IMPORTER_OPTION LOAD_MATERIAL  = 0x02;
            static const FBX_IMPORTER_OPTION LOAD_MESH      = 0x03; // polygon and material
            static const FBX_IMPORTER_OPTION LOAD_BONEWEIGHT = 0x04;
            static const FBX_IMPORTER_OPTION LOAD_ANIM      = 0x08;
            static const FBX_IMPORTER_OPTION LOAD_ALL       = 0x0f;
        };

    private:

        template <typename U>
        struct fbx_deleter { void operator()(U* p) const { p->Destroy(); } };
        
        // #[nodeName, nodeID]
        std::unordered_map<std::string, int> nodemap;
        
        SceneRaw rscene_;

        
        Mesh processMesh(const MeshRaw& src) {
            Mesh dst;
            std::vector<element_t> cache;
            cache.reserve(src.indices.size());

            dst.vertices.reserve(src.indices.size() * std::tuple_size<vertex_t>::value);
            dst.normals.reserve(src.indices.size() * std::tuple_size<normal_t>::value);
            dst.colors.reserve(src.indices.size() * std::tuple_size<color_t>::value);
            dst.uvs.reserve(src.indices.size() * std::tuple_size<uv_t>::value);
            dst.indices.reserve(src.indices.size());
            dst.boneIndices.reserve(src.indices.size() * std::tuple_size<bindex_t>::value);
            dst.boneWeights.reserve(src.indices.size() * std::tuple_size<bweight_t>::value);
            
            /* indexify */
            for (auto i = 0U; i < src.indices.size(); i++) {
                const auto& ver = src.vertices[i];
                const auto& nor = src.normals[i];
                const auto& uv = src.uvs[i];
                const auto& col = src.colors.empty() ? util::make_array<float>(1.f, 1.f, 1.f, 1.f) : src.colors[i];
                const auto& bi = src.boneIndices.empty() ? util::make_array<uint>(0U, 0U, 0U, 0U) : src.boneIndices[i];
                const auto& bw = src.boneWeights.empty() ? util::make_array<float>(0.f, 0.f, 0.f, 0.f) : src.boneWeights[i];
                
                auto it = std::find(
                    cache.begin(), 
                    cache.end(), 
                    std::make_tuple(ver, nor, col, uv, bi, bw)
                );
                if(it == cache.end()) {
                    /* not found */
                    for(auto&& v : ver) { dst.vertices.push_back(v); }
                    for(auto&& v : nor) { dst.normals.push_back(v); }
                    for(auto&& v : col) { dst.colors.push_back(v); }
                    for(auto&& v : uv) { dst.uvs.push_back(v); }
                    if(src.boneIndices.size()) {
                        for(auto&& v : bi) { dst.boneIndices.push_back(v); }
                    }
                    if(src.boneWeights.size()) {
                        for(auto&& v : bw) { dst.boneWeights.push_back(v); }
                    }
                    dst.indices.push_back(cache.size());
                    cache.emplace_back(ver, nor, col, uv, bi, bw);
                } else {
                    /* found */
                    const auto index = std::distance(cache.begin(), it);
                    dst.indices.push_back(index);
                }
            }
            assert(dst.indices.size() % 3 == 0);

            dst.texture = std::move(src.texture);

            return std::move(dst);
        }

        Anim processAnim(AnimRaw& src) {
            Anim dst;
            dst.meshes.reserve(src.meshes.size());
            for(auto&& m : src.meshes) {
                dst.meshes.push_back({std::move(m.meshMatrices), std::move(m.boneMatrices)});
            }
            return std::move(dst);
        }

        void processScene(Scene& dst, SceneRaw& src) {
            dst.meshes.reserve(src.meshes.size());
            dst.animes.reserve(src.animes.size());
            logger::info("process mesh...");
            for(auto&& src : src.meshes) {
                dst.meshes.push_back(std::move(processMesh(src)));
            }
            logger::info("process anim...");
            for(auto&& src : src.animes) {
                dst.animes.push_back(std::move(processAnim(src)));
            }
        }
        
        template <typename U, typename V>
        void parseElement(
            FbxMesh* const fbxmesh,
            const FbxLayerElementTemplate<U>* const el, 
            std::vector<V>& target, 
            const std::vector<unsigned int>& ind
        ) {
            const auto mapmode = el->GetMappingMode();
            const auto refmode = el->GetReferenceMode();
            const auto& indexArray = el->GetIndexArray();
            const auto& directArray = el->GetDirectArray();

            assert((refmode == FbxGeometryElement::eDirect) || (refmode == FbxGeometryElement::eIndexToDirect));
            assert((mapmode == FbxGeometryElement::eByPolygonVertex) || (mapmode == FbxGeometryElement::eByControlPoint));

            target.reserve(ind.size());

            auto f = [&](int i){
                auto d = directArray.GetAt((refmode == FbxGeometryElement::eDirect) ? i : indexArray.GetAt(i));
                V arr;
                for(auto i = 0U; i < std::tuple_size<V>::value; i++) { arr[i] = d[i]; }
                target.push_back(std::move(arr));
            };

            if(mapmode == FbxGeometryElement::eByControlPoint) {
                for(auto&& i : ind) { f(i); }
            } else {
                for(int i = 0; i < fbxmesh->GetPolygonCount() * 3; i++) { f(i); }
            }
        }

        
        void parseMesh(FbxMesh* const fbxmesh, MeshRaw& mesh) { 
            
            /* index */
            const auto pc = fbxmesh->GetPolygonCount();
            mesh.indices.reserve(pc * 3);
            for(int i = 0; i < pc; i++) {
                assert(fbxmesh->GetPolygonSize(i) == 3);
                mesh.indices.push_back(fbxmesh->GetPolygonVertex(i, 0));
                mesh.indices.push_back(fbxmesh->GetPolygonVertex(i, 1));
                mesh.indices.push_back(fbxmesh->GetPolygonVertex(i, 2));
            }
            
            /* vertex */
            mesh.vertices.reserve(mesh.indices.size());
            for(auto&& i : mesh.indices) {
                const auto cp = fbxmesh->GetControlPointAt(i);
                mesh.vertices.push_back(util::make_array<float>(
                    static_cast<float>(cp[0]),
                    static_cast<float>(cp[1]),
                    static_cast<float>(cp[2])));
                assert(cp[3] == 0.0);
            }

            /* normal */
            assert(fbxmesh->GetElementNormalCount() == 1);
            const auto enor = fbxmesh->GetElementNormal();
            parseElement(fbxmesh, enor, mesh.normals, mesh.indices);
            
            /* uv */
            assert(fbxmesh->GetElementUVCount() > 0);
            const auto euv = fbxmesh->GetElementUV(0);
            parseElement(fbxmesh, euv, mesh.uvs, mesh.indices);
            
            /* color */
            if(fbxmesh->GetElementVertexColorCount() > 0){
                const auto ec = fbxmesh->GetElementVertexColor();
                parseElement(fbxmesh, ec, mesh.colors, mesh.indices);
            }

        }

        void parseMaterial(FbxSurfaceMaterial* const material, MeshRaw& mesh) {
            const auto materialName = material->GetName();

            //logger::debug("material: ", materialName);

            // TODO: 
            auto implementation = GetImplementation(material, FBXSDK_IMPLEMENTATION_CGFX);
            if(!implementation) {
                implementation = GetImplementation(material, FBXSDK_IMPLEMENTATION_HLSL);
            }
            //assert(implementation != nullptr);
            
            // TODO: もうちょっと まとめる
            if(implementation == nullptr) {
                FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
                assert(prop.IsValid());
                const auto tc = prop.GetSrcObjectCount<FbxTexture>();
                if(tc > 0) {
                    const auto tfc = prop.GetSrcObjectCount<FbxFileTexture>();
                    //assert(tfc == 1);
                    //tfc = 1;
                    for(int k = 0; k < tfc; k++) {
                        const auto tex = prop.GetSrcObject<FbxFileTexture>(k);
                        std::string texName = tex->GetFileName();
                        texName = texName.substr(texName.find_last_of('/') + 1);
                        texName = texName.substr(texName.find_last_of('\\') + 1);
                        //std::cout << "[DEBUG] texture[" << "FbxSurfaceMaterial::sDiffuse" << "]: " << texName << std::endl;
                        if(k == 0) { mesh.texture = texName; }
                    }
                    const auto ltc = prop.GetSrcObjectCount<FbxLayeredTexture>();
                    for(int k = 0; k < ltc; k++) {
                        const auto lt = prop.GetSrcObject<FbxLayeredTexture>(k);
                        const auto tfc = lt->GetSrcObjectCount<FbxFileTexture>();
                        //assert(tfc == 1);
                        //tfc = 1;
                        for(int k = 0; k < tfc; k++) {
                            const auto tex = prop.GetSrcObject<FbxFileTexture>(k);
                            std::string texName = tex->GetFileName();
                            texName = texName.substr(texName.find_last_of('/') + 1);
                            texName = texName.substr(texName.find_last_of('\\') + 1);
                            //std::cout << "[DEBUG] layered texture[" << "FbxSurfaceMaterial::sDiffuse" << "]: " << texName << std::endl;
                            //mesh.texture = texName;
                        }
                    }
                }
                return;
            }
            
            const auto rt = implementation->GetRootTable();
           
            for(size_t i = 0; i < rt->GetEntryCount(); i++) {
                const auto entry = rt->GetEntry(i);
                std::string src = entry.GetSource();
                auto prop = material->FindPropertyHierarchical(src.c_str());
                if(!prop.IsValid()) {
                    prop = material->RootProperty.FindHierarchical(src.c_str());
                }
                const auto tc = prop.GetSrcObjectCount<FbxTexture>();
                if(tc > 0) {
                    const auto tfc = prop.GetSrcObjectCount<FbxFileTexture>();
                    assert(tfc == 1);
                    for(int k = 0; k < tfc; k++) {
                        const auto tex = prop.GetSrcObject<FbxFileTexture>(k);
                        std::string texName = tex->GetFileName();
                        texName = texName.substr(texName.find_last_of('/') + 1);
                        //std::cout << "[DEBUG] texture[" << src << "]: " << texName << std::endl;
                        // TODO:
                        if(mesh.texture.length() == 0){
                            mesh.texture = texName;
                        }
                        if(src == "Maya|DiffuseTexture"){
                            mesh.texture = texName;
                        }
                    }
                }
            }
        }

        void parseBoneWeight(FbxMesh* const fbxmesh, MeshRaw& mesh) {
            
            const auto sc = fbxmesh->GetDeformerCount(FbxDeformer::eSkin);
            if(sc == 0) {
                
                return;
            }
            assert(sc <= 1);

            using weightPair = std::pair<uint, float>;
            std::vector<std::vector<weightPair>> boneWeights(fbxmesh->GetControlPointsCount());
            const auto skin = static_cast<FbxSkin*>(fbxmesh->GetDeformer(0, FbxDeformer::eSkin));
            
            int cc = 0;
            for(int i = 0; i < skin->GetClusterCount(); i++){
                const auto cluster = skin->GetCluster(i);
                assert(cluster->GetLinkMode() == FbxCluster::eNormalize);
                
                const auto cpic = cluster->GetControlPointIndicesCount();
                if(cpic == 0) { continue; }

                const auto indices = cluster->GetControlPointIndices();
                const auto weights = cluster->GetControlPointWeights();
                for(int k = 0; k < cpic; k++) {
                    boneWeights[indices[k]].emplace_back(cc, static_cast<float>(weights[k]));
                }
                
                // save BoneNodeName
                mesh.boneNodeNames.push_back(cluster->GetLink()->GetName());

                // invMatrix of BasePose
                auto bbpm = cluster->GetLink()->EvaluateGlobalTransform().Inverse();
                mesh.invBoneBasePoseMatrices.push_back(bbpm);

                cc++;
            }

            // by control point
            std::vector<std::pair<bindex_t, bweight_t>> cpBone(boneWeights.size());
            std::transform(boneWeights.begin(), boneWeights.end(), cpBone.begin(), [&](auto& bw){
                // sort by weight
                std::sort(bw.begin(), bw.end(), [](auto& wp1, auto& wp2) {
                    return wp1.second > wp2.second;
                });
                while(bw.size() > 4) { bw.pop_back(); }
                while(bw.size() < 4) { bw.emplace_back(0, 0.f); }
                
                float total = 0.0f;
                bindex_t ind;
                bweight_t wei;
                for(int i = 0; i < 4; i++) {
                    ind[i] = bw[i].first;
                    total += wei[i] = bw[i].second;
                }
                // normalize
                for(int i = 0; i < 4; i++) {
                    wei[i] /= total;
                }
                
                return std::make_pair(ind, wei);
            });

            // extend by index
            mesh.boneIndices.reserve(mesh.indices.size());
            mesh.boneWeights.reserve(mesh.indices.size());
            for(auto i : mesh.indices) {
                mesh.boneIndices.push_back(cpBone[i].first);
                mesh.boneWeights.push_back(cpBone[i].second);
            }

        }

        void parseAnim(FbxScene* const fbxscene, const SceneRaw& scene, AnimRaw& anim) {
            
            anim.meshes.reserve(scene.meshes.size());

            for(auto&& m : scene.meshes) {
                AnimFrameRaw af;
                
                /* mesh matrix */
                auto itmesh = this->nodemap.find(m.nodeName);
                if(itmesh != this->nodemap.end()) {
                    af.meshMatrices.reserve(anim.end - anim.start + 1);
                    const auto meshNode = fbxscene->GetNode(itmesh->second);
                    for(auto frame = anim.start + 1; frame < anim.end - 1; frame++) {
                        FbxTime time;
                        time.Set(FbxTime::GetOneFrameValue(FbxTime::eFrames60) * frame);

                        const auto& meshMatrix = meshNode->EvaluateGlobalTransform(time);
                        //auto matrixRaw = m.invMeshBasePoseMatrix * meshMatrix;  f*ck
                        const auto matrixRaw = meshMatrix * m.invMeshBasePoseMatrix;
                        std::vector<float> matrix(16);
                        for(int i = 0; i < 16; i++) {
                            matrix[i] = static_cast<float>(matrixRaw[i / 4][i % 4]);
                        }
                        af.meshMatrices.push_back(std::move(matrix));

                    }
                }

                /* bone matrix */
                if(!m.boneNodeNames.empty()) {
                    std::vector<FbxNode*> boneNodes(m.boneNodeNames.size());
                    std::transform(m.boneNodeNames.begin(), m.boneNodeNames.end(), boneNodes.begin(), [&](auto bn){
                        auto it = this->nodemap.find(bn);
                        assert(it != this->nodemap.end());
                        return fbxscene->GetNode(it->second);
                    });
                    af.boneMatrices.reserve((anim.end - anim.start + 1));
                    
                    for(auto frame = anim.start + 1; frame < anim.end - 1; frame++) {
                        FbxTime time;
                        time.Set(FbxTime::GetOneFrameValue(FbxTime::eFrames60) * frame);

                        auto k = 0;
                        std::vector<float> boneMatrices;
                        boneMatrices.reserve(boneNodes.size());
                        for(auto&& boneNode : boneNodes) {
                            const auto& boneMatrix = boneNode->EvaluateGlobalTransform(time);
                            //auto matrixRaw = m.invBoneBasePoseMatrices[k] * boneMatrix;
                            const auto matrixRaw = (FbxMatrix)boneMatrix * m.invBoneBasePoseMatrices[k];
                            for(int i = 0; i < 16; i++) {
                                boneMatrices.push_back(static_cast<float>(matrixRaw[i / 4][i % 4]));
                            }
                            k++;
                        }
                        af.boneMatrices.push_back(std::move(boneMatrices));
                    }
                }
                anim.meshes.push_back(std::move(af));
            }
        }

        bool loadRaw(const char* const filename, SceneRaw& scene, FBX_IMPORTER_OPTION option = OPTION::LOAD_ALL) {

            logger::info("parse ", filename, " ...");

            std::unique_ptr<FbxManager, fbx_deleter<FbxManager>> manager(FbxManager::Create());

            auto ios = FbxIOSettings::Create(manager.get(), IOSROOT);
            //ex: ios->SetBoolProp(IMP_FBX_*, true);
            manager->SetIOSettings(ios);


            std::unique_ptr<FbxImporter, fbx_deleter<FbxImporter>> importer(FbxImporter::Create(manager.get(), ""));
            if(!importer->Initialize(filename, -1, manager->GetIOSettings())) {
                logger::error("[FBX Importer] ", importer->GetStatus().GetErrorString());
                return false;
            }

            std::unique_ptr<FbxScene, fbx_deleter<FbxScene>> fbxscene(FbxScene::Create(manager.get(), "Scene"));
            if(!importer->Import(fbxscene.get())) {
                logger::error("[FBX Importer] ", importer->GetStatus().GetErrorString());
                return false;
            }

            logger::debug("triangulate");
            FbxGeometryConverter geoconv(manager.get());
            if(!geoconv.Triangulate(fbxscene.get(), true)) {
                logger::warn("[WARN] triangulate failed.");
            }
            logger::debug("split material");
            if(!geoconv.SplitMeshesPerMaterial(fbxscene.get(), true)) {
                logger::warn("[WARN] split material failed.");
            }

            this->nodemap.clear();
            for(int i = 0; i < fbxscene->GetNodeCount(); i++) {
                const auto node = fbxscene->GetNode(i);
                this->nodemap.insert({ node->GetName(), i });
            }

            if(option & (OPTION::LOAD_MESH | OPTION::LOAD_BONEWEIGHT)) {

                const auto mc = fbxscene->GetMemberCount<FbxMesh>();
                for(int i = 0; i < mc; i++) {
                    const auto mesh = fbxscene->GetMember<FbxMesh>(i);

                    MeshRaw rmesh;
                    const auto node = mesh->GetNode();
                    rmesh.nodeName = node->GetName();
                    rmesh.invMeshBasePoseMatrix = node->EvaluateGlobalTransform().Inverse();
                    if(option & OPTION::LOAD_MATERIAL) {
                        const auto matc = node->GetMaterialCount();
                        // なんか形式によってはMeshがすべてのMaterialを持っていることがある？
                        // 同じMaterialを何度も参照してるので効率が悪い
                        // テクスチャ名しか取るつもりは無いのでまぁね
                        const auto mat = node->GetMaterial(mc == matc ? i : 0);
                        parseMaterial(mat, rmesh);
                    }
                    if(option & OPTION::LOAD_POLYGON) {
                        parseMesh(mesh, rmesh);
                    }
                    if(option & OPTION::LOAD_BONEWEIGHT) {
                        parseBoneWeight(mesh, rmesh);
                    }
                    scene.meshes.push_back(std::move(rmesh));
                }

            }
            if(option & OPTION::LOAD_ANIM) {
                assert(importer->GetAnimStackCount() == 1);

                AnimRaw ranim;
                const auto info = importer->GetTakeInfo(0);
                const auto offset = info->mImportOffset;
                const auto start = info->mLocalTimeSpan.GetStart();
                const auto stop = info->mLocalTimeSpan.GetStop();
                ranim.start = static_cast<int>((offset.Get() + start.Get()) / FbxTime::GetOneFrameValue(FbxTime::eFrames60));
                ranim.end = static_cast<int>((offset.Get() + stop.Get()) / FbxTime::GetOneFrameValue(FbxTime::eFrames60));

                parseAnim(fbxscene.get(), scene, ranim);

                scene.animes.push_back(std::move(ranim));
            }

            return true;
        }

    public:
        explicit FBXImporter() {}
        virtual ~FBXImporter() {}

        bool load(const char* const filename, FBX_IMPORTER_OPTION option = OPTION::LOAD_ALL) {
            return loadRaw(filename, rscene_, option);
        }

        bool load(const char* const filename, Scene& scene, FBX_IMPORTER_OPTION option = OPTION::LOAD_ALL) {
            if(!loadRaw(filename, rscene_, option)) { return false; }
            processScene(scene, rscene_);
            return true;
        }

    };

}} // namespace rhakt::rechor

#endif
