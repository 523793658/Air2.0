#include "ShaderFormatD3D.h"
#include "Interface/IShaderFormat.h"
#include "Interface/IShaderFormatModule.h"
#include "Containers/String.h"
#include "Modules/ModuleManager.h"
namespace Air
{


	static std::wstring NAME_PCD3D_SM5(TEXT("PCD3D_SM5"));
	static std::wstring NAME_PCD3D_SM4(TEXT("PCD3D_SM4"));
	static std::wstring NAME_PCD3D_ES3_1(TEXT("PCD3D_ES31"));
	static std::wstring NAME_PCD3D_ES2(TEXT("PCD3D_ES2"));
	
	class ShaderFormatD3D : public IShaderFormat
	{
		enum
		{
			AIR_SHADER_PCD3D_SM5_VER = 1,
			AIR_SHADER_PCD3D_SM4_VER = 1,
			AIR_SHADER_PCD3D_ES2_VER = 1,
			AIR_SHADER_PCD3D_ES3_1_VER = 1,
		};

		void checkFormat(std::wstring format)const
		{
			BOOST_ASSERT(format == NAME_PCD3D_SM5 || format == NAME_PCD3D_SM4 || format == NAME_PCD3D_ES2 || format == NAME_PCD3D_ES3_1);
		}
	public:
		virtual uint16 getVersion(wstring format) const override
		{
			checkFormat(format);
			if (format == NAME_PCD3D_SM5)
			{
				return AIR_SHADER_PCD3D_SM5_VER;
			}
			else if(format == NAME_PCD3D_SM4)
			{
				return AIR_SHADER_PCD3D_SM4_VER;
			}
			else if (format == NAME_PCD3D_ES2)
			{
				return AIR_SHADER_PCD3D_ES2_VER;
			}
			else if (format == NAME_PCD3D_ES3_1)
			{
				return AIR_SHADER_PCD3D_ES3_1_VER;
			}
			BOOST_ASSERT(false);
			return 0;
		}

		virtual void getSupportedFormats(TArray<wstring>& outFormats) const
		{
			outFormats.add(NAME_PCD3D_SM5);
			outFormats.add(NAME_PCD3D_SM4);
			outFormats.add(NAME_PCD3D_ES2);
			outFormats.add(NAME_PCD3D_ES3_1);
		}

		virtual void compileShader(wstring format, const struct ShaderCompilerInput& input, struct ShaderCompilerOutput& output, const wstring & workingDirectory) const
		{
			checkFormat(format);
			if (format == NAME_PCD3D_SM5)
			{
				compileShader_windows_sm5(input, output, workingDirectory);
			}
			else if (format == NAME_PCD3D_SM4)
			{
				compileShader_windows_sm4(input, output, workingDirectory);
			}
			else if (format == NAME_PCD3D_ES2)
			{
				compileShader_windows_es2(input, output, workingDirectory);
			}
			else if (format == NAME_PCD3D_ES3_1)
			{
				compileShader_windows_es3_1(input, output, workingDirectory);
			}
			else
			{
				BOOST_ASSERT(false);
			}
		}
	};

	static IShaderFormat* singleton = nullptr;
	class ShaderFormatD3DModule : public IShaderFormatModule
	{
	public:
		virtual ~ShaderFormatD3DModule()
		{
			delete singleton;
			singleton = nullptr;
		}

		virtual IShaderFormat* getShaderFormat()
		{
			if (!singleton)
			{
				singleton = new ShaderFormatD3D();
			}
			return singleton;
		}
	};

	IMPLEMENT_MODULE(ShaderFormatD3DModule, ShaderFormatD3D);
}

