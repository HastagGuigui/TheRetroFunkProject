#include "GDLevelLoader.h"
#include "../../../Encryption/Encryption.h"
#include "../../../String/StringAPI.h"
#include "../../../Web/Web.h"
#include "../../../Physics/Colliders/BoxCollider.h"
//#include "Graphics/Cores/Texture/rectpack2D/finders_interface.h"

void GDLevelLoader::OnStart()
{
	/*WebRequest request("http://www.boomlings.com/database/downloadGJLevel22.php", WebRequest::HttpMethod::HTTP_POST);
	WebResponse response;
	WebForm form;

	form.AddField("gameVersion", "20");
	form.AddField("binaryVersion", "35");
	form.AddField("gdw", "0");
	form.AddField("levelID", "1650666");
	form.AddField("secret", "Wmfd2893gb7");

	request.SendWebRequest(&response, form);

	std::string urlResult = response.body;*/

	GDTextures = TextureManager::OpenTexturePack("Game/Textures/Blocks/blocks.gtxp");

	//TextureManager::OpenSpriteSheet("Game/Textures/Blocks/GJ_GameSheet.plist");

	// Stereo Madness: 1556066
	// How by Spu7nix: 63395980
	// Future Funk by JonathanGD: 44062068
	// Object IDs by Colon: 58079690

	std::string urlResult = Web::Post("http://www.boomlings.com/database/downloadGJLevel22.php", "gameVersion=20&binaryVersion=35&gdw=0&levelID=1556066&secret=Wmfd2893gb7");

	std::string levelSubstring = StringAPI::GetSubstringBetween(urlResult, ":4:", ":5:");

	if (!StringAPI::StartsWith(levelSubstring, "kS"))
	{
		if (!StringAPI::StartsWith(levelSubstring, "-1") || levelSubstring != "")
		{
			//std::cout << levelSubstring << std::endl;
			std::string decodeBase64 = Encryption::Base64::Decode(levelSubstring, true);
			std::string decodeZLib = Encryption::ZLib::Inflate(decodeBase64, Encryption::ZLib::Format::GZIP);

			if (decodeZLib == "")
			{
				// This means the level is old, decode in ZLIB instead.
				decodeZLib = Encryption::ZLib::Inflate(decodeBase64, Encryption::ZLib::Format::ZLIB);
			}

			ReadLevel(decodeZLib);
		}
		else
		{
			std::cout << "Returned -1" << std::endl;
		}
	}
	else
	{
		ReadLevel(levelSubstring);
	}
}

void GDLevelLoader::ReadLevel(std::string rawData)
{
	std::vector<std::string> allObjects = StringAPI::SplitIntoVector(rawData, ";");
	LoadCustomProperties();

	TextureManager::UploadToGPU();

	for (auto i : allObjects)
	{
		if (!StringAPI::StartsWith(i, "kS"))
		{
			int objId = 0;
			Vector3 position, rotation, scale(1);
			bool fX = false, fY = false, noC = false;
			std::vector<std::string> allValues = StringAPI::SplitIntoVector(i, ",");
			for (int id = 0; id < allValues.size(); id += 2)
			{
				if (allValues[id] != "" && allValues[id + 1] != "")
				{
					int value = std::stoi(allValues[id]);
					float result = 0;
					try
					{
						result = std::stof(allValues[id + 1]);
					}
					catch (...)
					{

					}

					switch (value)
					{
					// Object ID
					case 1:
						objId = result;
						for (auto size : customSize)
						{
							if (size.first == result)
							{
								scale = size.second;
								break;
							}
						}

						for (auto collisions : noCollisions)
						{
							if (collisions == result)
							{
								noC = true;
								break;
							}
						}
						break;

					// X Position
					case 2:
						position.x = result / 30.0f;
						break;

					// Y Position
					case 3:
						position.y = result / 30.0f;
						break;

					//Flip X Texture
					case 4:
						if (allValues[id + 1] == "1")
							fX = true;
						break;

					//Flip Y Texture
					case 5:
						if (allValues[id + 1] == "1")
							fY = true;
						break;

					// Z Rotation
					case 6:
						rotation.z -= result;
						break;

					/*case 21:
						colorID = std::stoi(results[i]);
						break;*/

					// XY Scale
					case 32:
						if (scale.x != 0 && scale.y != 0)
						{
							scale.x *= result;
							scale.y *= result;
						}
						break;
					}
				}
			}
			
			Model* newObj = new Model(Model::Square(), position, rotation, scale);

			newObj->FlipTexture(fX, fY);

			if (!noC)
			{
				BoxCollider* b = newObj->AddScript<BoxCollider>();
				for (auto collSize : customCollisionSize)
				{
					if (collSize.first == objId)
					{
						b->SetScale(collSize.second);
					}
				}
			}

			newObj->SetTexture(*GDTextures[objId]);
			RendererCore::AddModel(*newObj);
		}
	}
}

void GDLevelLoader::LoadCustomProperties()
{
	//Custom Size
	//|
	//|
	//- Rods
	AddCustomSize(15, Vector3(0.21 * 1.5, 1.33 * 1.5, 1));
	AddCustomSize(16, Vector3(0.2 * 1.5, 0.88 * 1.5, 1));
	AddCustomSize(17, Vector3(0.18 * 1.5, 0.41 * 1.5, 1));
	//- Portals		[portal_XX_front_001.png]
	AddCustomSize(10,   Vector3(50.f / 60, 150.f / 60, 1));//gravity down		( 1)
	AddCustomSize(11,   Vector3(50.f / 60, 150.f / 60, 1));//gravity up			( 2)
	AddCustomSize(12,   Vector3(68.f / 60, 172.f / 60, 1));//cube				( 3)
	AddCustomSize(13,   Vector3(68.f / 60, 172.f / 60, 1));//ship				( 4)
	AddCustomSize(45,   Vector3(88.f / 60, 184.f / 60, 1));//mirror				( 5)
	AddCustomSize(46,   Vector3(88.f / 60, 184.f / 60, 1));//unmirror			( 6)
	AddCustomSize(47,   Vector3(68.f / 60, 172.f / 60, 1));//ball				( 7)
	AddCustomSize(99,   Vector3(62.f / 60, 180.f / 60, 1));//unmini				( 8)
	AddCustomSize(101,  Vector3(62.f / 60, 180.f / 60, 1));//mini				( 9)
	AddCustomSize(111,  Vector3(68.f / 60, 172.f / 60, 1));//ufo				(10)
	AddCustomSize(286,  Vector3(82.f / 60, 182.f / 60, 1));//dual mode			(11)
	AddCustomSize(287,  Vector3(82.f / 60, 182.f / 60, 1));//undual mode		(12)
	AddCustomSize(660,  Vector3(68.f / 60, 172.f / 60, 1));//wave				(13)
	AddCustomSize(745,  Vector3(68.f / 60, 172.f / 60, 1));//robot				(14)
	AddCustomSize(747,  Vector3(78.f / 60, 180.f / 60, 1));//teleport in		(15)
	AddCustomSize(749,  Vector3(78.f / 60, 180.f / 60, 1));//teleport out		(16)
	AddCustomSize(1331, Vector3(68.f / 60, 172.f / 60, 1));//robot				(17)
	//- decorations
	AddCustomSize(9,	Vector3(121.f / 120, 106.f / 120, 1));//pit_01_001.png
	AddCustomSize(1715, Vector3(121.f / 120, 106.f / 120, 1));//pit_01_001.png (2)
	//- Spikes
	//|-	1.0 spikes
	AddCustomSize(39,	Vector3(1, 54.f / 120, 1));			//spike_02_001.png
	AddCustomSize(103,	Vector3(80.f/120, 76.f / 120, 1));	//spike_03_001.png
	AddCustomSize(392,	Vector3(50.f/120, 48.f / 120, 1));	//spike_04_001.png
	//|-	invisible spikes
	AddCustomSize(205,	Vector3(1, 54.f / 120, 1));			//invis_spike_02_001.png
	AddCustomSize(145,	Vector3(80.f/120, 76.f / 120, 1));	//invis_spike_03_001.png
	AddCustomSize(459,	Vector3(50.f/120, 48.f / 120, 1));	//invis_spike_04_001.png
	//- slabs
	AddCustomSize(215,	Vector3(1, 54.f / 120, 1));
	AddCustomSize(40,	Vector3(1, 54.f / 120, 1));
	AddCustomSize(662,	Vector3(1, 60.f / 120, 1));
	//- Clouds		[d_cloud_XX_001.png]
	AddCustomSize(48,	Vector3(450.f / 120, 154.f / 120, 1));	//01
	AddCustomSize(49,	Vector3(320.f / 120, 128.f / 120, 1));	//02
	AddCustomSize(129,	Vector3(316.f / 120, 126.f / 120, 1));	//03
	AddCustomSize(130,	Vector3(458.f / 120, 176.f / 120, 1));	//04
	AddCustomSize(131,	Vector3(152.f / 120, 72.f  / 120, 1));	//05
	//- Deco spikes [d_spikes_XX_001.png]
	AddCustomSize(18,	Vector3(512.f / 120, 166.f / 120, 1));	//01
	AddCustomSize(19,	Vector3(416.f / 120, 146.f / 120, 1));	//02
	AddCustomSize(20,	Vector3(292.f / 120, 116.f / 120, 1));	//03
	AddCustomSize(21,	Vector3(168.f / 120, 52.f  / 120, 1));	//04

	// what the fuck rob

	//No Collisions
	//|
	//|
	//- Rods
	AddNoCollisions(15);
	AddNoCollisions(16);
	AddNoCollisions(17);
	//- Clouds
	AddNoCollisions(48);	//d_cloud_01_001.png
	AddNoCollisions(49);	//d_cloud_02_001.png
	AddNoCollisions(129);	//d_cloud_03_001.png
	AddNoCollisions(130);	//d_cloud_04_001.png
	AddNoCollisions(131);	//d_cloud_05_001.png
	//- Deco spikes
	AddNoCollisions(18);
	AddNoCollisions(19);
	AddNoCollisions(20);
	AddNoCollisions(21);

	//Custom Collision Size
	//|
	//|
	//- Spikes
	AddCustomCollisionSize(8, Vector3(0.2, 0.2, 1));
	AddCustomCollisionSize(39, Vector3(0.2, 0.2, 1));
	AddCustomCollisionSize(103, Vector3(0.2, 0.2, 1));
}

void GDLevelLoader::AddCustomSize(int id, Vector3 size)
{
	customSize.push_back(std::make_pair(id, size));
}

void GDLevelLoader::AddNoCollisions(int id)
{
	noCollisions.push_back(id);
}

void GDLevelLoader::AddCustomCollisionSize(int id, Vector3 size)
{
	customCollisionSize.push_back(std::make_pair(id, size));
}
