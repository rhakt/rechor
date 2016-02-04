// rechor project
// scene.hpp

#ifndef _RHACT_RECHOR_SCENE_HPP_
#define _RHACT_RECHOR_SCENE_HPP_

#include <vector>
#include <string>

namespace rechor {

    struct AnimFrame {
        std::vector<std::vector<float>> meshMatrices;
        std::vector<std::vector<float>> boneMatrices;
    };

    struct Anim {
        std::vector<AnimFrame> meshes;
    };

    struct Mesh {
        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<int> indices;
        std::vector<float> colors;
        std::vector<float> uvs;
        std::string texture;
        std::vector<int> boneIndices;
        std::vector<float> boneWeights;
    };

    struct Scene {
        std::vector<Mesh> meshes;
        std::vector<Anim> animes;
    };

} // namespace rechor

#endif
