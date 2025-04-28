#pragma once

#include "resource/Ref.h"
#include "serialize/yaml_util.h"
#include "resource/GenericAssetManager.h"
#include "reflection/SerializeTypes.h"

#include <string>
#include <filesystem>


namespace trace {

	class GenericSerializer
	{

	public:

		template<typename T>
		static bool Serialize(Ref<T> asset, const std::string& file_path)
		{

			if (!asset)
			{
				return false;
			}

			YAML::Emitter emit;

			emit << YAML::BeginMap;
			emit << YAML::Key << "Trace Version" << YAML::Value << "0.0.0.0";

			Reflection::Serialize(*asset.get(), &emit, nullptr, Reflection::SerializationFormat::YAML);


			emit << YAML::EndMap;

			YAML::save_emitter_data(emit, file_path);

			return true;

		}

		template<typename T>
		static Ref<T> Deserialize(const std::string& file_path)
		{
			Ref<T> result;

			std::filesystem::path p = file_path;
			Ref<T> asset = GenericAssetManager::get_instance()->Get<T>(p.filename().string());
			if (asset)
			{
				TRC_WARN("{} has already been loaded", p.filename().string());
				return asset;
			}

			YAML::Node data;
			if (!YAML::load_yaml_data(file_path, data))
			{
				return result;
			}

			if (!data["Trace Version"] )
			{
				TRC_ERROR("These file is not a valid asset file {}", file_path);
				return result;
			}

			std::string trace_version = data["Trace Version"].as<std::string>(); // TODO: To be used later
			std::string asset_name = p.filename().string();

			result = GenericAssetManager::get_instance()->CreateAssetHandle_<T>(file_path);

			Reflection::Deserialize(*result.get(), &data, nullptr, Reflection::SerializationFormat::YAML);


			return result;
		}

		template<typename T>
		static bool Serialize(Ref<T> asset, DataStream* stream)
		{
			if (!asset)
			{
				return false;
			}

			std::string asset_name = asset->GetName();
			Reflection::Serialize(asset_name, stream, nullptr, Reflection::SerializationFormat::BINARY);
			Reflection::Serialize(*asset.get(), stream, nullptr, Reflection::SerializationFormat::BINARY);

			return true;
		}

		template<typename T>
		static Ref<T> Deserialize(DataStream* stream)
		{
			std::string asset_name;
			Reflection::Deserialize(asset_name, stream, nullptr, Reflection::SerializationFormat::BINARY);
			Ref<T> asset = GenericAssetManager::get_instance()->Get<T>(asset_name);
			if (asset)
			{
				TRC_WARN("{} has already been loaded", asset_name);
				return asset;
			}


			asset = GenericAssetManager::get_instance()->CreateAssetHandle_<T>(asset_name);

			Reflection::Deserialize(*asset.get(), stream, nullptr, Reflection::SerializationFormat::BINARY);

			return asset;
		}

	private:

	protected:

	};



}
