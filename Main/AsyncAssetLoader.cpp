#include "stdafx.h"
#include "AsyncAssetLoader.hpp"
#include "Application.hpp"

struct AsyncLoadOperation : public IAsyncLoadable
{
	String name;
};
struct AsyncTextureLoadOperation : public AsyncLoadOperation
{
	shared_ptr<Texture>& target;
	unique_ptr<Image> image;
	AsyncTextureLoadOperation(shared_ptr<Texture>& target, const String& path) : target(target)
	{
		name = path;
	}
	bool AsyncLoad()
	{
		image = g_application->LoadImage(name);
		return true;
	}
	bool AsyncFinalize()
	{
		auto newtex = Texture::Create(*image);
		if (!newtex)
			return false;
		target = std::move(*newtex);
		return true;
	}
};
struct AsyncMeshLoadOperation : public AsyncLoadOperation
{
	shared_ptr<Mesh>& target;
	AsyncMeshLoadOperation(shared_ptr<Mesh>& target, const String& path) : target(target)
	{
	}
	bool AsyncLoad()
	{
		/// TODO: No mesh loading yet
		return false;
	}
	bool AsyncFinalize()
	{
		/// TODO: No mesh loading yet
		return false;
	}
}; 
struct AsyncMaterialLoadOperation : public AsyncLoadOperation
{
	shared_ptr<Material>& target;
	AsyncMaterialLoadOperation(shared_ptr<Material>& target, const String& path) : target(target)
	{
		name = path;
	}
	bool AsyncLoad()
	{
		return true;
	}
	bool AsyncFinalize()
	{
		target = g_application->LoadMaterial(name);
		return true;
	}
};
struct AsyncWrapperOperation : public AsyncLoadOperation
{
	IAsyncLoadable& target;
	AsyncWrapperOperation(IAsyncLoadable& target, const String& name) : target(target)
	{
		this->name = name;
	}
	bool AsyncLoad()
	{
		return target.AsyncLoad();
	}
	bool AsyncFinalize()
	{
		return target.AsyncFinalize();
	}
};

class AsyncAssetLoader_Impl
{
public:
	Vector<AsyncLoadOperation*> loadables;
	~AsyncAssetLoader_Impl()
	{
		for(auto& loadable : loadables)
		{
			delete loadable;
		}
	}
};

AsyncAssetLoader::AsyncAssetLoader()
{
	m_impl = new AsyncAssetLoader_Impl();
}
AsyncAssetLoader::~AsyncAssetLoader()
{
	delete m_impl;
}

void AsyncAssetLoader::AddTexture(shared_ptr<Texture>& out, const String& path)
{
	m_impl->loadables.Add(new AsyncTextureLoadOperation(out, path));
}
void AsyncAssetLoader::AddMesh(shared_ptr<Mesh>& out, const String& path)
{
	m_impl->loadables.Add(new AsyncMeshLoadOperation(out, path));
}
void AsyncAssetLoader::AddMaterial(shared_ptr<Material>& out, const String& path)
{
	m_impl->loadables.Add(new AsyncMaterialLoadOperation(out, path));
}
void AsyncAssetLoader::AddLoadable(IAsyncLoadable& loadable, const String& id /*= "unknown"*/)
{
	m_impl->loadables.Add(new AsyncWrapperOperation(loadable, id));
}

bool AsyncAssetLoader::Load()
{
	bool success = true;
	for(auto& ld : m_impl->loadables)
	{
		if(!ld->AsyncLoad())
		{
			Logf("[AsyncLoad] Load failed on %s", Logger::Error, ld->name);
			success = false;
		}
	}
	return success;
}
bool AsyncAssetLoader::Finalize()
{
	bool success = true;
	for(auto& ld : m_impl->loadables)
	{
		if(!ld->AsyncFinalize())
		{
			Logf("[AsyncLoad] Finalize failed on %s", Logger::Error, ld->name);
			success = false;
		}
	}

	// Clear state
	delete m_impl;
	m_impl = new AsyncAssetLoader_Impl();

	return success;
}
