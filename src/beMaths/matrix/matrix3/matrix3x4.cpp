#include "matrix3x4.hpp"

#include "matrix.hpp" // IWYU pragma: keep
#include "vector.hpp" // IWYU pragma: keep

#include "beCore.hpp"

namespace be{

/**
 * An empty constructor
*/
Matrix3x4::Matrix3x4()
    : Matrix3x4({
        std::array<float, 4>({0.f, 0.f, 0.f, 0.f}),
        std::array<float, 4>({0.f, 0.f, 0.f, 0.f}),
        std::array<float, 4>({0.f, 0.f, 0.f, 0.f})
    }){}

/**
 * A basic constructor
 * @param v The value for all the elements in the matrix
*/
Matrix3x4::Matrix3x4(float v)
    : Matrix3x4({
        std::array<float, 4>({v,v,v,v}),
        std::array<float, 4>({v,v,v,v}),
        std::array<float, 4>({v,v,v,v})
    }){}

/**
 * Copy a matrix
 * @param matrix The matrix to cpoy
*/
Matrix3x4& Matrix3x4::operator=(const Matrix3x4& matrix){
    copy(matrix);
    return *this;
}

/**
 * A constructor by copy
 * @param matrix The matrix to copy
*/
Matrix3x4::Matrix3x4(const Matrix3x4& matrix){
    copy(matrix);
}

/**
 * Copy a matrix
 * @param matrix The matrix to copy
*/
void Matrix3x4::copy(const Matrix3x4& matrix){
    for(size_t i = 0; i<matrix._Values.size(); i++){
        for(size_t j = 0; j<matrix._Values[0].size(); j++){
            _Values[i][j] = matrix._Values[i][j];
        }
    }
}

/**
 * A basic constructor
 * @param values The array containing the values
*/
Matrix3x4::Matrix3x4(const std::array<std::array<float, 4>, 3>& values){
    _Values = values;
}

/**
 * Create a matrix fill with ones
*/
Matrix3x4 Matrix3x4::ones(){
    return Matrix3x4(1.f);
}

/**
 * Create a matrix fill with zeros
*/
Matrix3x4 Matrix3x4::zeros(){
    return Matrix3x4();
}

/**
 * Cast the matrix into a string
 * @return The string
*/
const std::string Matrix3x4::toString() const{
    std::string str = "{";
    for(size_t i = 0; i<_Values.size(); i++){
        str += "\n{" + std::to_string(_Values[i][0]);
        for(size_t j = 1; j<_Values[0].size(); j++){
            str += ", " + std::to_string(_Values[i][j]);
        }
        str += "}";
    }
    return str;
}

/**
 * Getter for the matrices elements
 * @param index The index of the element to access
 * @return The wanted value
*/
std::array<float, 4> Matrix3x4::operator[](int index) const{
    if(index < 0 || index >= static_cast<int>(_Values.size())){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR, 
            "Index " + std::to_string(index) + " out of range for Matrix3x4!\n",
            ErrorLevel::WARNING
        );
    }
    return _Values[index];
}

/**
 * Setter for the matrices elements
 * @param index The index of the element to set
 * @return The wanted value
*/
std::array<float, 4>& Matrix3x4::operator[](int index){
    if(index < 0 || index >= static_cast<int>(_Values.size())){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR, 
            "Index " + std::to_string(index) + " out of range for Matrix3x4!\n",
            ErrorLevel::WARNING
        );
    }
    return _Values[index];
}

/**
 * Addition between two matrices
 * @param matrix The second matrix
 * @return The sum of the two matrices
*/
Matrix3x4 Matrix3x4::operator+(const Matrix3x4& matrix) const{
    Matrix3x4 newMat{};
    for(size_t i = 0; i<_Values.size(); i++){
        for(size_t j = 0; j<_Values[0].size(); j++){
            newMat._Values[i][j] = _Values[i][j] + matrix[i][j];
        }
    }
    return newMat;
}

/**
 * Substraction between two matrices
 * @param matrix The second matrix
 * @return The substraction of the two matrices
*/
Matrix3x4 Matrix3x4::operator-(const Matrix3x4& matrix) const{
    Matrix3x4 newMat{};
    for(size_t i = 0; i<_Values.size(); i++){
        for(size_t j = 0; j<_Values[0].size(); j++){
            newMat._Values[i][j] = _Values[i][j] - matrix[i][j];
        }
    }
    return newMat;
}

/**
 * Addition between two matrices
 * @param matrix The second matrix
*/
void Matrix3x4::operator+=(const Matrix3x4& matrix){
    for(size_t i = 0; i<_Values.size(); i++){
        for(size_t j = 0; j<_Values[0].size(); j++){
            _Values[i][j] += matrix[i][j];
        }
    }
}

/**
 * Substraction between two matrices
 * @param matrix The second matrix
*/
void Matrix3x4::operator-=(const Matrix3x4& matrix){
    for(size_t i = 0; i<_Values.size(); i++){
        for(size_t j = 0; j<_Values[0].size(); j++){
            _Values[i][j] -= matrix[i][j];
        }
    }
}

/**
 * Multiplication with a scalar
 * @param val The scalar
*/
void Matrix3x4::operator*=(float scalar){
    for(size_t i = 0; i<_Values.size(); i++){
        for(size_t j = 0; j<_Values[0].size(); j++){
            _Values[i][j] *= scalar;
        }
    }
}

/**
 * Division with a scalar
 * @param val The scalar
*/
void Matrix3x4::operator/=(float scalar){
    if(scalar == 0.f){
        ErrorHandler::handle(
            ErrorCode::ZERO_DIVIDE_ERROR, 
            "Cannot divide by 0!\n",
            ErrorLevel::WARNING
        );
        return;
    }
    for(size_t i = 0; i<_Values.size(); i++){
        for(size_t j = 0; j<_Values[0].size(); j++){
            _Values[i][j] /= scalar;
        }
    }
}

/**
 * Multiplication between two matrices
 * @param matrix The second matrix
*/
void Matrix3x4::operator*=(const Matrix4x4& matrix){
    Matrix3x4 newMat{};
    for(size_t i = 0; i<_Values.size(); i++){
        for(size_t j = 0; j<_Values[0].size(); j++){
            for (size_t k = 0; k < _Values.size(); k++) {
                newMat[i][j] += _Values[i][k] * matrix[k][j];
            }
        }
    }
    copy(newMat);
}

/**
 * Multiplication with a vector
 * @param vector The vector
 * @return The resulting scalar
*/
Vector3 Matrix3x4::operator*(const Vector4& vector) const{
    Vector3 newVector{};
    for(size_t i = 0; i<_Values.size(); i++){
        for(size_t j = 0; j<_Values[0].size(); j++){
            newVector[i] += _Values[i][j]*vector[j];
        }
    }
    return newVector;
}

/**
 * Multiplication between two matrices
 * @param matrix The second matrix
 * @return The resulting matrix
*/
Matrix3x2 Matrix3x4::operator*(const Matrix4x2& matrix) const{
    Matrix3x2 newMat{};
    for(size_t i = 0; i<_Values.size(); i++){
        for(size_t j = 0; j<2; j++){
            for (size_t k = 0; k < _Values[0].size(); k++) {
                newMat[i][j] += _Values[i][k] * matrix[k][j];
            }
        }
    }
    return newMat;
}

/**
 * Multiplication between two matrices
 * @param matrix The second matrix
 * @return The resulting matrix
*/
Matrix3x3 Matrix3x4::operator*(const Matrix4x3& matrix) const{
    Matrix3x3 newMat{};
    for(size_t i = 0; i<_Values.size(); i++){
        for(size_t j = 0; j<3; j++){
            for (size_t k = 0; k < _Values[0].size(); k++) {
                newMat[i][j] += _Values[i][k] * matrix[k][j];
            }
        }
    }
    return newMat;
}

/**
 * Multiplication between two matrices
 * @param matrix The second matrix
 * @return The resulting matrix
*/
Matrix3x4 Matrix3x4::operator*(const Matrix4x4& matrix) const{
    Matrix3x4 newMat{};
    for(size_t i = 0; i<_Values.size(); i++){
        for(size_t j = 0; j<4; j++){
            for (size_t k = 0; k < _Values[0].size(); k++) {
                newMat[i][j] += _Values[i][k] * matrix[k][j];
            }
        }
    }
    return newMat;
}

}