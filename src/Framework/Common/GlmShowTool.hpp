#pragma once
#include <iostream>
#include <glm/glm.hpp>
template<int size,typename T = float>
void ShowMat(const glm::mat <size, size,T, glm::qualifier::defaultp > & mat)
{
	glm::mat4();
	static_assert(size > 0);
	for (uint32_t i = 0; i < size; i++)
	{
		std::cout << std::endl;
		for (uint32_t j = 0; j < size; j++)
		{
			std::cout << mat[i][j] << " ";
		}
	}
	std::cout << std::endl;
}
template<int size_row,int size_col, typename T = float>
void ShowMat(const glm::mat <size_row, size_col, T, glm::qualifier::defaultp >& mat)
{
	glm::mat4();
	//static_assert()
	static_assert(size_row > 0 && size_col > 0);
	for (uint32_t i = 0; i < size_row; i++)
	{
		std::cout << std::endl;
		for (uint32_t j = 0; j < size_col; j++)
		{
			std::cout << mat[i][j] << "      ";
		}
	}
}

template<int size_row, int size_col, typename T = float>
void ShowMatColMajor(const glm::mat <size_row, size_col, T, glm::qualifier::defaultp >& mat)
{
	glm::mat4();
	//static_assert()
	static_assert(size_row > 0 && size_col > 0);
	for (uint32_t i = 0; i < size_col; i++)
	{
		std::cout << std::endl;
		for (uint32_t j = 0; j < size_row; j++)
		{
			std::cout << mat[j][i] << "      ";
		}
	}
}


template<int size_row, int size_col, typename T = float>
void TrimMat(glm::mat <size_row, size_col, T, glm::qualifier::defaultp >& mat)
{
	glm::mat4();
	//static_assert()
	static_assert(size_row > 0 && size_col > 0);
	for (uint32_t i = 0; i < size_row; i++)
	{
		std::cout << std::endl;
		for (uint32_t j = 0; j < size_col; j++)
		{
			if (fabs(mat[i][j]) < 0.000001)
			{
				mat[i][j] = 0.0;
			}
		}
	}
}


template<int size_row, int size_col, typename T = float>
void CompareMatEqual(const glm::mat <size_row, size_col, T, glm::qualifier::defaultp >& mat1, const glm::mat <size_row, size_col, T, glm::qualifier::defaultp >& mat2)
{
	glm::mat4();
	//static_assert()
	static_assert(size_row > 0 && size_col > 0);
	for (uint32_t i = 0; i < size_row; i++)
	{
		std::cout << std::endl;
		for (uint32_t j = 0; j < size_col; j++)
		{
			if (fabs(mat1[i][j] - mat2[i][j]) > 0.00001)
			{
				std::cout << " Not equal " << "      ";
				return;
			}
			
		}
	}
}


template<int size, typename T = float>
void ShowVec(const glm::vec <size, T, glm::qualifier::defaultp >& vec)
{
	glm::mat4();
	static_assert(size > 0);
	for (uint32_t i = 0; i < size; i++)
	{
		std::cout << std::endl;
		std::cout << vec[i] << " ";
	}
	std::cout << std::endl;
}
