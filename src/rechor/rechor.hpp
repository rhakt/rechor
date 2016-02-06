// rechor project
// rechor.hpp

#ifndef _RHACT_RECHOR_RECHOR_HPP_
#define _RHACT_RECHOR_RECHOR_HPP_

#include <vector>
#include <string>

#include <lz4.h>

#include "../logger.hpp"
#include "../util.hpp"
#include "scene_generated.h"

namespace rhakt {
namespace rechor {

    typedef unsigned int uint;
    typedef unsigned char uchar;

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


}} // namespace rhakt::rechor

#endif
