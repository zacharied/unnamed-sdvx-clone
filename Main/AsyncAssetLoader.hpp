#pragma once
#include "AsyncLoadable.hpp"

/*
	Loads assets and IAsyncLoadables 
	Acts like a queue that stores loading commands
*/
class AsyncAssetLoader : public Unique
{
public:
	AsyncAssetLoader();
	~AsyncAssetLoader();

	/// NOTE: the caller is responsible for keeping the passed in variable valid the this object is destroyed or finished with the loading
	// Add a texture to be loaded
	void AddTexture(shared_ptr<Texture>& out, const String& path);
	// Add a mesh to be loaded
	void AddMesh(shared_ptr<Mesh>& out, const String& path);
	// Add a mesh to be loaded
	void AddMaterial(shared_ptr<Material>& out, const String& path);
	// Add a loadable to be loaded, additionaly with a name so it can be identified in logs if it fails loading
	void AddLoadable(IAsyncLoadable& loadable, const String& id = "unknown");

	bool Load();
	bool Finalize();

private:
	class AsyncAssetLoader_Impl* m_impl;
};