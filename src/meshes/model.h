#pragma once

#include"utils/debug.h"
#include"libraries/include/json/json.h"
#include"Mesh.h"

using json = nlohmann::json;

std::string get_file_contents(const char* filename);

class Model
{
public:
    // Constructor for unloaded model
    Model();
    // Constructor if we already have loaded data
    Model(const char* file, unsigned int instancing = 1, std::vector<glm::mat4> instanceMatrix = {});

    // Loads in a model from a file and stores tha information in 'data', 'JSON', and 'file'
    void loadModel(const char* file, unsigned int instances = 1, std::vector<glm::mat4> instanceMatrix = {});

    // Function to update the instance matrices (say, if you want a new random allotment of stuff)
    void updateInstances(unsigned int new_instances, std::vector<glm::mat4> newInstanceMatrix);

    void Draw
        (
            Shader& shader,
            glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f),
            glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f)
            );

    // Frees all associated OpenGL memory with this model
    // TODO: Implement
    void cleanup();

private:
    // Variables for easy access
    const char* file;
    std::vector<unsigned char> data;
    json JSON;

    // Determines if model has been instantiated or not (cannot be drawn otherwise)
    bool instantiated = false;

    // Holds number of instances (if 1 the mesh will be rendered normally)
    unsigned int instances;

    // All the meshes and transformations
    std::vector<Mesh> meshes;
    std::vector<glm::vec3> translationsMeshes;
    std::vector<glm::quat> rotationsMeshes;
    std::vector<glm::vec3> scalesMeshes;
    std::vector<glm::mat4> matricesMeshes;
    std::vector<glm::mat4> instanceMatrix;

    // Prevents textures from being loaded twice
    std::vector<std::string> loadedTexName;
    std::vector<Texture> loadedTex;

    // Loads a single mesh by its index
    void loadMesh(unsigned int indMesh);

    // Traverses a node recursively, so it essentially traverses all connected nodes
    void traverseNode(unsigned int nextNode, glm::mat4 matrix = glm::mat4(1.0f));

    // Gets the binary data from a file
    std::vector<unsigned char> getData();
    // Interprets the binary data into floats, indices, and textures
    std::vector<float> getFloats(json accessor);
    std::vector<GLuint> getIndices(json accessor);
    std::vector<Texture> getTextures();

    // Assembles all the floats into vertices
    std::vector<Vertex> assembleVertices
        (
            std::vector<glm::vec3> positions,
            std::vector<glm::vec3> normals,
            std::vector<glm::vec2> texUVs
            );

    // Helps with the assembly from above by grouping floats
    std::vector<glm::vec2> groupFloatsVec2(std::vector<float> floatVec);
    std::vector<glm::vec3> groupFloatsVec3(std::vector<float> floatVec);
    std::vector<glm::vec4> groupFloatsVec4(std::vector<float> floatVec);
};
