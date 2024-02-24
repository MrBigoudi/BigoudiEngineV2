#include "be_componentMaterial.hpp"
#include "be_errorHandler.hpp"

namespace be{

const std::array<std::string, ComponentMaterial::COMPONENT_MATERIAL_NB_ELEMENTS> 
    ComponentMaterial::COMPONENT_MATERIAL_NAMES{
        "Metallic",
        "Subsurface",
        "Specular",
        "Roughness",
        "SpecularTint",
        "Anisotropic",
        "Sheen",
        "SheenTint",
        "Clearcoat",
        "ClearcoatGloss",
};

const std::array<float, ComponentMaterial::COMPONENT_MATERIAL_NB_ELEMENTS> 
    ComponentMaterial::COMPONENT_MATERIAL_MIN_VALUES{
        0.f, // metallic
        0.f, // subsurface
        0.f, // specular
        0.f, // roughness
        0.f, // specularTint
        0.f, // anisotropic
        0.f, // sheen
        0.f, // sheenTint
        0.f, // clearcoat
        0.f, // clearcoatGloss
};

const std::array<float, ComponentMaterial::COMPONENT_MATERIAL_NB_ELEMENTS> 
    ComponentMaterial::COMPONENT_MATERIAL_MAX_VALUES{
        1.f, // metallic
        1.f, // subsurface
        1.f, // specular
        1.f, // roughness
        1.f, // specularTint
        1.f, // anisotropic
        1.f, // sheen
        1.f, // sheenTint
        1.f, // clearcoat
        1.f, // clearcoatGloss
};

void ComponentMaterial::check() const {
    uint32_t i = 0;

    if(_Metallic < COMPONENT_MATERIAL_MIN_VALUES[i]
        && _Metallic > COMPONENT_MATERIAL_MAX_VALUES[i]
    ){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Wrong material mettalicness: " 
            + std::to_string(_Metallic) 
            + "!\n"
        );
    }
    i++;

    if(_Subsurface < COMPONENT_MATERIAL_MIN_VALUES[i]
        && _Subsurface > COMPONENT_MATERIAL_MAX_VALUES[i]
    ){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Wrong material subsurface: " 
            + std::to_string(_Subsurface) 
            + "!\n"
        );
    }
    i++;

    if(_Specular < COMPONENT_MATERIAL_MIN_VALUES[i]
        && _Specular > COMPONENT_MATERIAL_MAX_VALUES[i]
    ){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Wrong material specular: " 
            + std::to_string(_Specular) 
            + "!\n"
        );
    }
    i++;

    if(_Roughness < COMPONENT_MATERIAL_MIN_VALUES[i]
        && _Roughness > COMPONENT_MATERIAL_MAX_VALUES[i]
    ){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Wrong material roughness: " 
            + std::to_string(_Roughness) 
            + "!\n"
        );
    }
    i++;

    if(_SpecularTint < COMPONENT_MATERIAL_MIN_VALUES[i]
        && _SpecularTint > COMPONENT_MATERIAL_MAX_VALUES[i]
    ){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Wrong material specular tint: " 
            + std::to_string(_SpecularTint) 
            + "!\n"
        );
    }
    i++;

    if(_Anisotropic < COMPONENT_MATERIAL_MIN_VALUES[i]
        && _Anisotropic > COMPONENT_MATERIAL_MAX_VALUES[i]
    ){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Wrong material anisotropy: " 
            + std::to_string(_Anisotropic) 
            + "!\n"
        );
    }
    i++;

    if(_Sheen < COMPONENT_MATERIAL_MIN_VALUES[i]
        && _Sheen > COMPONENT_MATERIAL_MAX_VALUES[i]
    ){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Wrong material sheen: " 
            + std::to_string(_Sheen) 
            + "!\n"
        );
    }
    i++;

    if(_SheenTint < COMPONENT_MATERIAL_MIN_VALUES[i]
        && _SheenTint > COMPONENT_MATERIAL_MAX_VALUES[i]
    ){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Wrong material sheen tint: " 
            + std::to_string(_SheenTint) 
            + "!\n"
        );
    }
    i++;

    if(_Clearcoat < COMPONENT_MATERIAL_MIN_VALUES[i]
        && _Clearcoat > COMPONENT_MATERIAL_MAX_VALUES[i]
    ){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Wrong material clearcoat: " 
            + std::to_string(_Clearcoat) 
            + "!\n"
        );
    }
    i++;

    if(_ClearcoatGloss < COMPONENT_MATERIAL_MIN_VALUES[i]
        && _ClearcoatGloss > COMPONENT_MATERIAL_MAX_VALUES[i]
    ){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Wrong material clearcoat gloss: " 
            + std::to_string(_ClearcoatGloss) 
            + "!\n"
        );
    }
    i++;

}

float& ComponentMaterial::get(uint32_t id){
    if(id > COMPONENT_MATERIAL_NB_ELEMENTS){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Index " + std::to_string(id) 
            + "is too high, material indices can't be greater than " 
            + std::to_string(COMPONENT_MATERIAL_NB_ELEMENTS)
            + " !\n"
        );
    }
    switch(id){
        case 0:
            return _Metallic;
        case 1:
            return _Subsurface;
        case 2:
            return _Specular;
        case 3:
            return _Roughness;
        case 4:
            return _SpecularTint;
        case 5:
            return _Anisotropic;
        case 6:
            return _Sheen;
        case 7:
            return _SheenTint;
        case 8:
            return _Clearcoat;
        case 9:
            return _ClearcoatGloss;
        default:
            return _Metallic;
    }
}

};