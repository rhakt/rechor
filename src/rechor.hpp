// rechor project
// rechor.hpp

#ifndef _RHACT_RECHOR_RECHOR_HPP_
#define _RHACT_RECHOR_RECHOR_HPP_

#include <vector>
#include <string>

#include <flatbuffers/util.h>
#include <lz4.h>

#include "scene.hpp"
#include "logger.hpp"
#include "util.hpp"
#include "scene_generated.h"

namespace rechor {

    typedef unsigned int uint;

    class Importer : private Noncopyable {
    public:
        explicit Importer() {}
        virtual ~Importer() {}

        bool load(const char* filename, Scene& scene) {
            
            logger::info("loading...");
            
            std::string buf;
            if(!flatbuffers::LoadFile(filename, true, &buf)) {
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
                mm->vertices();
                for(auto&& v : *(mm->vertices())) {
                    mesh.vertices.push_back(v);
                }
                for(auto&& v : *(mm->normals())) {
                    mesh.normals.push_back(v);
                }
                for(auto&& v : *(mm->indices())) {
                    mesh.indices.push_back(v);
                }
                for(auto&& v : *(mm->colors())) {
                    mesh.colors.push_back(v);
                }
                for(auto&& v : *(mm->uvs())) {
                    mesh.uvs.push_back(v);
                }
                mesh.texture = mm->texture()->str();

                scene.meshes.push_back(std::move(mesh));
            }
            

            /* TODO: import bone, anim */

            return true;
        }
    };

    class Exporter : private Noncopyable {
    private:
        flatbuffers::FlatBufferBuilder fbb;
    
    public:
        explicit Exporter() {}
        virtual ~Exporter() {}

        bool save(const char* filename, const Scene& scene, bool binary = true) {

            logger::info("saving...");

            std::vector<flatbuffers::Offset<model::Mesh>> mm(scene.meshes.size());
            std::vector<flatbuffers::Offset<model::Anim>> aa;

            std::transform(scene.meshes.begin(), scene.meshes.end(), mm.begin(), [&](auto& m){
                auto vertex = fbb.CreateVector(m.vertices);
                auto normal = fbb.CreateVector(m.normals);
                auto index = fbb.CreateVector(m.indices);
                auto color = fbb.CreateVector(m.colors);
                auto uv = fbb.CreateVector(m.uvs);
                auto tex = fbb.CreateString(m.texture);
                auto bi = fbb.CreateVector(m.boneIndices);
                auto bw = fbb.CreateVector(m.boneWeights);
                model::MeshBuilder mb(fbb);
                mb.add_vertices(vertex);
                mb.add_normals(normal);
                mb.add_indices(index);
                mb.add_colors(color);
                mb.add_uvs(uv);
                mb.add_texture(tex);
                mb.add_boneIndices(bi);
                mb.add_boneWeights(bw);
                return std::move(mb.Finish());
            });

            for(auto&& a : scene.animes) {
                std::vector<flatbuffers::Offset<model::AnimFrame>> af;
                for(auto&& m : a.meshes) {
                    std::vector<flatbuffers::Offset<model::Frame>> mmf;
                    for(auto&& mf : m.meshMatrices) {
                        auto data = fbb.CreateVector(mf);
                        model::FrameBuilder fb(fbb);
                        fb.add_data(data);
                        mmf.push_back(std::move(fb.Finish()));
                    }
                    std::vector<flatbuffers::Offset<model::Frame>> bif;
                    for(auto&& bf : m.boneMatrices) {
                        auto data = fbb.CreateVector(bf);
                        model::FrameBuilder fb(fbb);
                        fb.add_data(data);
                        bif.push_back(std::move(fb.Finish()));
                    }
                    auto vmmf = fbb.CreateVector(mmf);
                    auto vbif = fbb.CreateVector(bif);
                    model::AnimFrameBuilder afb(fbb);
                    afb.add_meshMatrices(vmmf);
                    afb.add_boneMatrices(vbif);
                    af.push_back(std::move(afb.Finish()));
                }
                auto ms = fbb.CreateVector(af);
                model::AnimBuilder ab(fbb);
                ab.add_meshes(ms);
                aa.push_back(std::move(ab.Finish()));
            }
            auto mesh = fbb.CreateVector(mm);
            auto anim = fbb.CreateVector(aa);

            model::SceneBuilder sb(fbb);
            sb.add_meshes(mesh);
            sb.add_animes(anim);

            model::FinishSceneBuffer(fbb, sb.Finish());

            const auto inputsize = fbb.GetSize();
            std::unique_ptr<char> dest(new char[LZ4_compressBound(inputsize)]);
            auto outputsize = LZ4_compress(reinterpret_cast<const char *>(fbb.GetBufferPointer()), dest.get(), inputsize);
            if(outputsize <= 0) {
                logger::error("[LZ4] compress error");
                return false;
            }

            auto ok = flatbuffers::SaveFile(
                filename,
                dest.get(),
                outputsize,
                binary
                );

            fbb.ReleaseBufferPointer();
            
            if(!ok) {
                logger::error("[Flatbuffers] SaveFile error");
            }
            return ok;
        }
    };

} // namespace rechor

#endif
