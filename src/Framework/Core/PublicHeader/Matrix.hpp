#pragma once

#include <array>
#include <cstddef>
#include <type_traits>

namespace Core
{

    // ============================================================
    // Matrix Template
    // Row-major storage (like glm by default)
    // ============================================================

    template <typename T, std::size_t Rows, std::size_t Cols>
    class Matrix
    {
        static_assert(Rows > 0 && Cols > 0, "Matrix dimensions must be > 0");
        static_assert(std::is_arithmetic_v<T>, "Matrix scalar type must be arithmetic");

    public:
        using ScalarType = T;
        static constexpr std::size_t RowCount = Rows;
        static constexpr std::size_t ColCount = Cols;

        // Row-major storage: data[row][col]
        std::array<T, Rows* Cols> data{};

    public:
        // ------------------------------------------------------------
        // Constructors
        // ------------------------------------------------------------

        constexpr Matrix() = default;

        constexpr explicit Matrix(T value)
        {
            for (std::size_t i = 0; i < Rows * Cols; ++i)
                data[i] = value;
        }

        // Construct from initializer list (row-major)
        constexpr Matrix(std::initializer_list<T> init)
        {
            std::size_t i = 0;
            for (T v : init)
            {
                if (i < Rows * Cols)
                    data[i++] = v;
            }
        }

        // ------------------------------------------------------------
        // Element Access
        // ------------------------------------------------------------

        constexpr T& operator()(std::size_t row, std::size_t col)
        {
            return data[row * Cols + col];
        }

        constexpr const T& operator()(std::size_t row, std::size_t col) const
        {
            return data[row * Cols + col];
        }

        constexpr T* Data() { return data.data(); }
        constexpr const T* Data() const { return data.data(); }

        // ------------------------------------------------------------
        // Identity (only for square matrices)
        // ------------------------------------------------------------

        static constexpr Matrix Identity()
        {
            static_assert(Rows == Cols, "Identity matrix must be square");

            Matrix result{};
            for (std::size_t i = 0; i < Rows; ++i)
                result(i, i) = static_cast<T>(1);
            return result;
        }
    };

    // ============================================================
    // Common Type Aliases (glm-like naming)
    // ============================================================

    // float matrices
    using Mat3 = Matrix<float, 3, 3>;
    using Mat3x4 = Matrix<float, 3, 4>;
    using Mat4 = Matrix<float, 4, 4>;

    // explicit-sized aliases (engine friendly)
    using Mat3f = Matrix<float, 3, 3>;
    using Mat3x4f = Matrix<float, 3, 4>;
    using Mat4f = Matrix<float, 4, 4>;

    using Mat3d = Matrix<double, 3, 3>;
    using Mat3x4d = Matrix<double, 3, 4>;
    using Mat4d = Matrix<double, 4, 4>;

} // namespace math