#pragma once

#include "be_ubo.hpp"
#include <array>
#include <cstdint>
#include <string>
#include <memory>

namespace be{

/**
 * Forward declaration of the material structure
 * @see Material
*/
struct Material;

/**
 * Smart pointer to a material
 * @see Material
*/
using MaterialPtr = std::shared_ptr<Material>;

/**
 * The maximum number of material in a scene
*/
static const int MAX_NB_MATERIALS = 10;

/**
 * A class to store game objects material
 * @note This class is based on disney's brdfs implementation
 * @see https://github.com/wdas/brdf/blob/main/src/brdfs/disney.brdf
*/
struct Material {
    /**
    * The metallicness value
    * @note default to 0
    * @note minimum 0
    * @note maximum 1
    */
    alignas(16) float _Metallic = 0.f;

    /**
    * The subsurface value
    * @note default to 0
    * @note minimum 0
    * @note maximum 1
    */
    float _Subsurface = 0.f;

    /**
    * The specular value
    * @note default to 0
    * @note minimum 0
    * @note maximum 1
    */
    float _Specular = 0.f;

    /**
    * The roughness value
    * @note default to 0
    * @note minimum 0
    * @note maximum 1
    */
    float _Roughness = 0.f;

    /**
    * The specular tint
    * @note default to 0
    * @note minimum 0
    * @note maximum 1
    */
    float _SpecularTint = 0.f;

    /**
    * The anisotropy
    * @note default to 0
    * @note minimum 0
    * @note maximum 1
    */
    float _Anisotropic = 0.f;

    /**
    * The sheen value
    * @note default to 0
    * @note minimum 0
    * @note maximum 1
    */
    float _Sheen = 0.f;

    /**
    * The sheen tint
    * @note default to 0
    * @note minimum 0
    * @note maximum 1
    */
    float _SheenTint = 0.f;

    /**
    * The clearcoat value
    * @note default to 0
    * @note minimum 0
    * @note maximum 1
    */
    float _Clearcoat = 0.f;

    /**
    * The clearcoat gloss
    * @note default to 0
    * @note minimum 0
    * @note maximum 1
    */
    float _ClearcoatGloss = 0.f;

    public:
        // EXTREMUM VALUES
        /**
        * The number of material elements
        */
        static const uint32_t COMPONENT_MATERIAL_NB_ELEMENTS = 10;

        /**
        * The name of each element of the material
        */
        static const std::array<std::string, COMPONENT_MATERIAL_NB_ELEMENTS> COMPONENT_MATERIAL_NAMES;

        /**
        * The minimum value for each element of the material
        */
        static const std::array<float, COMPONENT_MATERIAL_NB_ELEMENTS> COMPONENT_MATERIAL_MIN_VALUES;

        /**
        * The maximum value for each element of the material
        */
        static const std::array<float, COMPONENT_MATERIAL_NB_ELEMENTS> COMPONENT_MATERIAL_MAX_VALUES;

    public:
        /**
        * Check if all values are correct
        */
        void check() const;

        /**
        * Get a reference to the wanted value
        * @param id The index of the wanted value
        * @return A reference to this value
        */
        float& get(uint32_t id);

        /**
         * Convert a meterial into a string
         * @return A std::string
        */
        std::string toString() const;
};

struct MaterialUboData: public UboData{
    Material _Materials[MAX_NB_MATERIALS];
};

class MaterialUboContainer: public UboContainer{
    private:
        MaterialUboData _UboData{};

    public:
        virtual void init(uint32_t size, VulkanAppPtr vulkanApp) override;
        virtual void update(uint32_t frameIndex) override;

        void setMaterial(MaterialPtr material, uint32_t id);
        MaterialUboData getData() const{return _UboData;}
};

}