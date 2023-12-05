#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <iostream>

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    // Load the initial camera data and global data constants
    renderData.cameraData = fileReader.getCameraData();
    renderData.globalData = fileReader.getGlobalData();

    // Prepare to traverse the scene graph for shapes and lights
    renderData.shapes.clear();
    renderData.lights.clear();

    glm::mat4 identity(1.0f);

    dfsRenderShapes(fileReader.getRootNode(), identity, renderData);
    return true;
}

// Helper function to recursively traverse transform tree
// Makes objects of inherited primitive types using smart pointers!
void SceneParser::dfsRenderShapes(SceneNode *curr_node, glm::mat4 ctm, RenderData &render_data) {
    // Base case for empty node
    if (!curr_node) {
        return;
    }

    // For each transform (in reverse order), multiply current CTM by that transform
    for (int i = 0; i < curr_node->transformations.size(); i++) {
        glm::mat4 transform;
        // Switch case to generate the right kind of transform
        switch(curr_node->transformations[i]->type) {
        case(TransformationType::TRANSFORMATION_TRANSLATE):
            transform = glm::translate(curr_node->transformations[i]->translate);
            break;
        case(TransformationType::TRANSFORMATION_SCALE):
            transform = glm::scale(curr_node->transformations[i]->scale);
            break;
        case(TransformationType::TRANSFORMATION_ROTATE):
            transform = glm::rotate(curr_node->transformations[i]->angle, curr_node->transformations[i]->rotate);
            break;
        case(TransformationType::TRANSFORMATION_MATRIX):
            transform = curr_node->transformations[i]->matrix;
            break;
        }

        // Perform the multiplication!
        ctm = ctm * transform;
    }

    // For each light, construct and append a SceneLightData using the new CLM
    for (int i = 0; i < curr_node->lights.size(); i++) {
        SceneLightData new_light;

        // Copy over old field data
        new_light.id = curr_node->lights[i]->id;
        new_light.type = curr_node->lights[i]->type;
        new_light.function = curr_node->lights[i]->function;
        new_light.color = curr_node->lights[i]->color;
        new_light.penumbra = curr_node->lights[i]->penumbra;
        new_light.angle = curr_node->lights[i]->angle;
        new_light.width = curr_node->lights[i]->width;
        new_light.height = curr_node->lights[i]->height;

        // Compute new position and direction
        new_light.dir = ctm * curr_node->lights[i]->dir;
        glm::vec4 init_pos(0.0f, 0.0f, 0.0f, 1.0f);
        new_light.pos = ctm * init_pos;

        render_data.lights.push_back(new_light);
    }

    // For each shape, construct and append a RenderShapeData using the new CLM
    for (int i = 0; i < curr_node->primitives.size(); i++) {
        // Create a new shape data to add
        RenderShapeData new_shape;

        // Copy over old field data
        new_shape.primitive.type = curr_node->primitives[i]->type;
        new_shape.primitive.material = curr_node->primitives[i]->material;
        new_shape.primitive.meshfile = curr_node->primitives[i]->meshfile;
        new_shape.ctm = ctm;

        // Create object depending on field type
        // NOTE: You can remove this switch case later (or just check for not mesh)
        if (curr_node->primitives[i]->type != PrimitiveType::PRIMITIVE_MESH) {
            // Initialize primitive as long as it isn't mesh
            render_data.shapes.push_back(new_shape);
        }
        else {
            throw(std::runtime_error("Rendering mesh objects not supported. Exiting..."));
        }
    }

    // For each child, recurse! Make sure to pass CLM as a value, and renderData as a reference
    for (int i = 0; i < curr_node->children.size(); i++) {
        dfsRenderShapes(curr_node->children[i], ctm, render_data);
    }
}
