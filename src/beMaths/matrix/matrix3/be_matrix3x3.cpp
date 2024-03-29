#include "be_matrix3x3.hpp"

#include "be_matrix.hpp" // IWYU pragma: keep
#include "be_vector.hpp" // IWYU pragma: keep

#include "be_errorHandler.hpp"

namespace be{

/**
 * An empty constructor
*/
Matrix3x3::Matrix3x3()
    : Matrix3x3({
        std::array<float, 3>({0.f, 0.f, 0.f}),
        std::array<float, 3>({0.f, 0.f, 0.f}),
        std::array<float, 3>({0.f, 0.f, 0.f})
    }){}

/**
 * A basic constructor
 * @param v The value for all the elements in the matrix
*/
Matrix3x3::Matrix3x3(float v)
    : Matrix3x3({
        std::array<float, 3>({v,v,v}),
        std::array<float, 3>({v,v,v}),
        std::array<float, 3>({v,v,v})
    }){}

/**
 * A constructor by copy
 * @param matrix The matrix to copy
*/
Matrix3x3::Matrix3x3(const Matrix3x3& matrix){
    copy(matrix);
}

/**
 * Copy a matrix
 * @param matrix The matrix to cpoy
*/
Matrix3x3& Matrix3x3::operator=(const Matrix3x3& matrix){
    copy(matrix);
    return *this;
}

/**
 * Copy a matrix
 * @param matrix The matrix to copy
*/
void Matrix3x3::copy(const Matrix3x3& matrix){
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
Matrix3x3::Matrix3x3(const std::array<std::array<float, 3>, 3>& values){
    _Values = values;
}

/**
 * Create a matrix fill with ones
*/
Matrix3x3 Matrix3x3::ones(){
    return Matrix3x3(1.f);
}

/**
 * Create a matrix fill with zeros
*/
Matrix3x3 Matrix3x3::zeros(){
    return Matrix3x3();
}

/**
 * Create a diagonal matrix
 * @param v The value for all the elements in the diagonal
*/
Matrix3x3 Matrix3x3::diag(float v){
    Matrix3x3 newMat{};
    for(size_t i = 0; i<newMat._Values.size(); i++){
        newMat._Values[i][i] = v;
    }
    return newMat;
} 

/**
 * Create an identity matrix
*/
Matrix3x3 Matrix3x3::identity(){
    return diag(1.f);
}

/**
 * Cast the matrix into a string
 * @return The string
*/
const std::string Matrix3x3::toString() const{
    std::string str = "{";
    for(size_t i = 0; i<_Values.size(); i++){
        str += "\n{" + std::to_string(_Values[i][0]);
        for(size_t j = 1; j<_Values[0].size(); j++){
            str += ", " + std::to_string(_Values[i][j]);
        }
        str += "}";
    }
    str += "\n}";
    return str;
}

/**
 * Getter for the matrices elements
 * @param index The index of the element to access
 * @return The wanted value
*/
std::array<float, 3> Matrix3x3::operator[](int index) const{
    if(index < 0 || index >= static_cast<int>(_Values.size())){
        ErrorHandler::handle(__FILE__, __LINE__, 
            ErrorCode::BAD_VALUE_ERROR, 
            "Index " + std::to_string(index) + " out of range for Matrix3x3!\n",
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
std::array<float, 3>& Matrix3x3::operator[](int index){
    if(index < 0 || index >= static_cast<int>(_Values.size())){
        ErrorHandler::handle(__FILE__, __LINE__, 
            ErrorCode::BAD_VALUE_ERROR, 
            "Index " + std::to_string(index) + " out of range for Matrix3x3!\n",
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
Matrix3x3 Matrix3x3::operator+(const Matrix3x3& matrix) const{
    Matrix3x3 newMat{};
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
Matrix3x3 Matrix3x3::operator-(const Matrix3x3& matrix) const{
    Matrix3x3 newMat{};
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
void Matrix3x3::operator+=(const Matrix3x3& matrix){
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
void Matrix3x3::operator-=(const Matrix3x3& matrix){
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
void Matrix3x3::operator*=(float scalar){
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
void Matrix3x3::operator/=(float scalar){
    if(scalar == 0.f){
        ErrorHandler::handle(__FILE__, __LINE__, 
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
void Matrix3x3::operator*=(const Matrix3x3& matrix){
    Matrix3x3 newMat{};
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
Vector3 Matrix3x3::operator*(const Vector3& vector) const{
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
Matrix3x2 Matrix3x3::operator*(const Matrix3x2& matrix) const{
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
Matrix3x3 Matrix3x3::operator*(const Matrix3x3& matrix) const{
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
Matrix3x4 Matrix3x3::operator*(const Matrix3x4& matrix) const{
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

/**
 * Get the determinant of the current matrix
 * @return The determinant as a float
*/
float Matrix3x3::det() const {
    Matrix2x2 matA = Matrix2x2({
        std::array<float, 2>({_Values[1][1], _Values[1][2]}), 
        std::array<float, 2>({_Values[2][1], _Values[2][2]})
    });

    Matrix2x2 matB = Matrix2x2({
        std::array<float, 2>({_Values[1][0], _Values[1][2]}), 
        std::array<float, 2>({_Values[2][0], _Values[2][2]})
    });

    Matrix2x2 matC = Matrix2x2({
        std::array<float, 2>({_Values[1][0], _Values[1][1]}), 
        std::array<float, 2>({_Values[2][0], _Values[2][1]})
    });

    return _Values[0][0] * matA.det()
        - _Values[0][1] * matB.det()
        + _Values[0][2] * matC.det();
}

/**
 * Get the determinant of the given matrix
 * @param matrix The given matrix
 * @return The determinant as a float
*/
float Matrix3x3::det(const Matrix3x3& matrix){
    return matrix.det();
}

/**
 * Transpose the current matrix
*/
void Matrix3x3::transpose(void){
    for(int i=0; i<4; i++){
        for(int j=0; j<i; j++){
            float oldValue = _Values[i][j];
            _Values[i][j] = _Values[j][i];
            _Values[j][i] = oldValue; 
        }
    }
}

/**
 * Transpose the current matrix
 * @param matrix The matrix to transpose
 * @return The transposed matrix
*/
Matrix3x3 Matrix3x3::transpose(const Matrix3x3& matrix){
    Matrix3x3 newMat(matrix);
    newMat.transpose();
    return newMat;
}

/**
 * Inverse a the current matrix
*/
void Matrix3x3::inverse(){

    float det = this->det();
    if(det == 0){
        ErrorHandler::handle(
            __FILE__, __LINE__,
            ErrorCode::ZERO_DIVIDE_ERROR,
            "Matrix is not inversible!\n"
        );
    }
    det = 1.f / det;

    Matrix3x3 tmp{};
    tmp[0][0] = det * (_Values[1][1] * _Values[2][2] - _Values[2][1] * _Values[1][2]);
    tmp[0][1] = det * (_Values[0][2] * _Values[2][1] - _Values[0][1] * _Values[2][2]);
    tmp[0][2] = det * (_Values[0][1] * _Values[1][2] - _Values[0][2] * _Values[1][1]);
    tmp[1][0] = det * (_Values[1][2] * _Values[2][0] - _Values[1][0] * _Values[2][2]);
    tmp[1][1] = det * (_Values[0][0] * _Values[2][2] - _Values[0][2] * _Values[2][0]);
    tmp[1][2] = det * (_Values[1][0] * _Values[0][2] - _Values[0][0] * _Values[1][2]);
    tmp[2][0] = det * (_Values[1][0] * _Values[2][1] - _Values[2][0] * _Values[1][1]);
    tmp[2][1] = det * (_Values[2][0] * _Values[0][1] - _Values[0][0] * _Values[2][1]);
    tmp[2][2] = det * (_Values[0][0] * _Values[1][1] - _Values[1][0] * _Values[0][1]);

    copy(tmp);
}

/**
 * Inverse the given matrix
 * @param matrix The matrix to vinert
 * @return The inverted matrix
*/
Matrix3x3 Matrix3x3::inverse(const Matrix3x3& matrix){
    Matrix3x3 newMat(matrix);
    newMat.inverse();
    return newMat; 
}

Matrix3x3 Matrix3x3::getCrossProductMatrix(const Vector3& v){
    Matrix3x3 out{};
    out[0][1] = -v.z();
    out[0][2] = v.y();
    out[1][0] = v.z();
    out[1][2] = -v.x();
    out[2][0] = -v.y();
    out[2][1] = v.x();
    return out;
}


}