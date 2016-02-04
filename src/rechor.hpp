// rechor project
// rechor.hpp

#ifndef _RHACT_RECHOR_RECHOR_HPP_
#define _RHACT_RECHOR_RECHOR_HPP_

#include <vector>
#include <string>

#include <flatbuffers/util.h>
#include <lz4.h>

#include "scene.hpp"
#include "util.hpp"
#include "scene_generated.h"

namespace rechor {

    typedef unsigned int uint;

    class Importer : private Noncopyable {
    public:
        explicit Importer() {}
        virtual ~Importer() {}

        bool load(const char* filename, Scene& scene) {
            std::string buf;
            if(!flatbuffers::LoadFile(filename, true, &buf)) {
                return false;
            }

            const auto inputsize = buf.size();
            std::unique_ptr<char> dest(new char[inputsize * 10]);
            auto outputsize = LZ4_decompress_safe(buf.data(), dest.get(), inputsize, inputsize * 10);
            if (outputsize <= 0) {
                std::cout << "[LZ4] error." << std::endl;
            }

            auto s = model::GetScene(reinterpret_cast<const void*>(dest.get()));
            
            auto m = s->meshes();

            for(auto&& mm : *m) {
                Mesh m;
                for(auto&& v : *(mm->vertices())) {
                    m.vertices.push_back(v);
                }
                for (auto&& v : *(mm->normals())) {
                    m.normals.push_back(v);
                }
                for (auto&& v : *(mm->indices())) {
                    m.indices.push_back(v);
                }
                for (auto&& v : *(mm->colors())) {
                    m.colors.push_back(v);
                }
                for (auto&& v : *(mm->uvs())) {
                    m.uvs.push_back(v);
                }
                m.texture = mm->texture()->str();
                scene.meshes.emplace_back(m);
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

            std::vector<flatbuffers::Offset<model::Mesh>> mm;
            std::vector<flatbuffers::Offset<model::Anim>> aa;

            for(auto&& m : scene.meshes) {
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
                mm.push_back(mb.Finish());
            }
            for(auto&& a : scene.animes) {
                std::vector<flatbuffers::Offset<model::AnimFrame>> af;
                for(auto&& m : a.meshes) {
                    std::vector<flatbuffers::Offset<model::Frame>> mmf;
                    for(auto&& mf : m.meshMatrices) {
                        auto data = fbb.CreateVector(mf);
                        model::FrameBuilder fb(fbb);
                        fb.add_data(data);
                        mmf.push_back(fb.Finish());
                    }
                    std::vector<flatbuffers::Offset<model::Frame>> bif;
                    for(auto&& bf : m.boneMatrices) {
                        auto data = fbb.CreateVector(bf);
                        model::FrameBuilder fb(fbb);
                        fb.add_data(data);
                        bif.push_back(fb.Finish());
                    }
                    auto vmmf = fbb.CreateVector(mmf);
                    auto vbif = fbb.CreateVector(bif);
                    model::AnimFrameBuilder afb(fbb);
                    afb.add_meshMatrices(vmmf);
                    afb.add_boneMatrices(vbif);
                    af.push_back(afb.Finish());
                }
                auto ms = fbb.CreateVector(af);
                model::AnimBuilder ab(fbb);
                ab.add_meshes(ms);
                aa.push_back(ab.Finish());
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
                std::cout << "[LZ4] error." << std::endl;
            }

            auto ok = flatbuffers::SaveFile(
                filename,
                dest.get(),
                outputsize,
                binary
                );

            fbb.ReleaseBufferPointer();
           
            return ok;
        }
    };

} // namespace rechor

#endif
