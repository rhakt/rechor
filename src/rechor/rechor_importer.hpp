// rechor project
// rechor_importer.hpp

#ifndef _RHACT_RECHOR_RECHOR_IMPORTER_HPP_
#define _RHACT_RECHOR_RECHOR_IMPORTER_HPP_

#include <vector>
#include <string>

#include <lz4.h>

#include "rechor.hpp"

namespace rhakt {
namespace rechor {

    class Importer : private util::Noncopyable {
    public:
        explicit Importer() {}
        virtual ~Importer() {}

        bool load(const char* filename, Scene& scene) {
            
            logger::info("loading...");
            
            std::string buf;
            if(!util::loadfile(filename, true, buf)) {
                logger::error("[Flatbuffers] LoadFile error");
                return false;
            }

            const auto inputsize = buf.size();
            std::unique_ptr<char> dest(new char[inputsize * 10]);
            auto outputsize = LZ4_decompress_safe(buf.data(), dest.get(), inputsize, inputsize * 10);
            if (outputsize <= 0) {
                logger::error("[LZ4] decompress error");
                return false;
            }

            auto s = model::GetScene(reinterpret_cast<const void*>(dest.get()));
            
            auto m = s->meshes();
            scene.meshes.reserve(m->size());
            for(auto&& mm : *m) {
                Mesh mesh;
                // TODO: ‰˜‚¢
                mesh.vertices.reserve(mm->vertices()->size());
                for(auto&& v : *(mm->vertices())) {
                    mesh.vertices.push_back(v);
                }

                mesh.normals.reserve(mm->normals()->size());
                for(auto&& v : *(mm->normals())) {
                    mesh.normals.push_back(v);
                }

                mesh.indices.reserve(mm->indices()->size());
                for(auto&& v : *(mm->indices())) {
                    mesh.indices.push_back(v);
                }

                mesh.colors.reserve(mm->colors()->size());
                for(auto&& v : *(mm->colors())) {
                    mesh.colors.push_back(v);
                }

                mesh.uvs.reserve(mm->uvs()->size());
                for(auto&& v : *(mm->uvs())) {
                    mesh.uvs.push_back(v);
                }
                
                mesh.texture = mm->texture()->str();
                
                mesh.boneIndices.reserve(mm->boneIndices()->size());
                for(auto&& v : *(mm->boneIndices())) {
                    mesh.boneIndices.push_back(v);
                }

                mesh.boneWeights.reserve(mm->boneWeights()->size());
                for(auto&& v : *(mm->boneWeights())) {
                    mesh.boneWeights.push_back(v);
                }

                scene.meshes.push_back(std::move(mesh));
            }
            
            auto a = s->animes();
            scene.animes.reserve(a->size());
            for(auto&& aa : *a) {
                Anim anim;
                auto af = aa->meshes();
                anim.meshes.reserve(af->size());
                for(auto&& aaa : *af) {
                    AnimFrame anf;
                    auto mm = aaa->meshMatrices();
                    anf.meshMatrices.reserve(mm->size());
                    for(auto&& mmm : *mm) {
                        auto data = mmm->data();
                        std::vector<float> v;
                        v.reserve(data->size());
                        for(auto&& d : *data) {
                            v.push_back(d);
                        }
                        anf.meshMatrices.push_back(std::move(v));
                    }
                    auto bm = aaa->boneMatrices();
                    anf.boneMatrices.reserve(bm->size());
                    for(auto&& bmm : *bm) {
                        auto data = bmm->data();
                        std::vector<float> v;
                        v.reserve(data->size());
                        for(auto&& d : *data) {
                            v.push_back(d);
                        }
                        anf.boneMatrices.push_back(std::move(v));
                    }
                    anim.meshes.push_back(std::move(anf));
                }
                scene.animes.push_back(std::move(anim));
            }
            
            return true;
        }
    };

}} // namespace rhakt::rechor

#endif
