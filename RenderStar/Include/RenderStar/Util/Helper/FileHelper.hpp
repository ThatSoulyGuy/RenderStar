#pragma once

#include "RenderStar/Util/Core/String.hpp"
#include "RenderStar/Util/Typedefs.hpp"

#undef CreateDirectory

using namespace RenderStar::Util::Core;

namespace RenderStar
{
	namespace Util
	{
		namespace Helper
		{
			class FileHelper
			{

			public:

				FileHelper(const FileHelper&) = delete;
				FileHelper& operator=(const FileHelper&) = delete;

				bool FileExists(const String& path) const
				{
					InputFileStream file(path.operator std::string());

					return file.good();
				}

				bool DirectoryExists(const String& path) const
				{
					return std::filesystem::exists(path.operator std::string());
				}

				bool CreateDirectory(const String& path) const
				{
					return std::filesystem::create_directory(path.operator std::string());
				}

				bool CreateDirectories(const String& path) const
				{
					return std::filesystem::create_directories(path.operator std::string());
				}

				bool Remove(const String& path) const
				{
					return std::filesystem::remove(path.operator std::string());
				}

				bool RemoveAll(const String& path) const
				{
					return std::filesystem::remove_all(path.operator std::string());
				}

				void Rename(const String& oldPath, const String& newPath) const
				{
					std::filesystem::rename(oldPath.operator std::string(), newPath.operator std::string());
				}

				void Copy(const String& sourcePath, const String& destinationPath) const
				{
					std::filesystem::copy(sourcePath.operator std::string(), destinationPath.operator std::string());
				}

				void Copy(const String& sourcePath, const String& destinationPath, std::filesystem::copy_options options) const
				{
					std::filesystem::copy(sourcePath.operator std::string(), destinationPath.operator std::string(), options);
				}

				void CopyDirectory(const String& sourcePath, const String& destinationPath) const
				{
					std::filesystem::copy(sourcePath.operator std::string(), destinationPath.operator std::string(), std::filesystem::copy_options::recursive);
				}

				void CopyDirectory(const String& sourcePath, const String& destinationPath, std::filesystem::copy_options options) const
				{
					std::filesystem::copy(sourcePath.operator std::string(), destinationPath.operator std::string(), options);
				}

				void Move(const String& sourcePath, const String& destinationPath) const
				{
					std::filesystem::rename(sourcePath.operator std::string(), destinationPath.operator std::string());
				}

				void MoveDirectory(const String& sourcePath, const String& destinationPath) const
				{
					std::filesystem::rename(sourcePath.operator std::string(), destinationPath.operator std::string());
				}

				void ResizeFile(const String& path, size_t newSize) const
				{
					std::filesystem::resize_file(path.operator std::string(), newSize);
				}

				void ResizeFile(const String& path, size_t newSize, std::error_code& errorCode) const
				{
					std::filesystem::resize_file(path.operator std::string(), newSize, errorCode);
				}

				String ReadFile(const String& path) const
				{
					InputFileStream fileStream(path.operator std::string());

					if (!fileStream)
					{
						Logger_ThrowException("Could not open file: " + path.operator std::string(), false);
						return "";
					}

					OutputStringStream stringStream;

					stringStream << fileStream.rdbuf();

					return stringStream.str();
				}

				static Shared<FileHelper> GetInstance()
				{
					class Enabled : public FileHelper { };
					static Shared<FileHelper> instance = std::make_shared<Enabled>();

					return instance;
				}

			private:

				FileHelper() = default;

			};
		}
	}
}