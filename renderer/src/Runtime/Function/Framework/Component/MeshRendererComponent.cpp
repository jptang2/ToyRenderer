#include "MeshRendererComponent.h"
#include "Core/Math/Math.h"
#include "Function/Framework/Component/TransformComponent.h"
#include "Function/Framework/Component/TryGetComponent.h"
#include "Function/Global/EngineContext.h"
#include "Resource/Asset/Asset.h"
#include <cstdint>
#include <memory>

CEREAL_REGISTER_TYPE(MeshRendererComponent)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, MeshRendererComponent)

MeshRendererComponent::~MeshRendererComponent()
{
    if(!EngineContext::Destroyed()) 
    {
        for(auto& objectID : objectIDs) EngineContext::RenderResource()->ReleaseObjectID(objectID); 
        objectIDs.clear();

        for(auto& meshCardID : meshCardIDs) 
        {
            EngineContext::Render()->GetSurfaceCacheManager()->ReleaseCache(meshCardID);
            EngineContext::RenderResource()->ReleaseMeshCardID(meshCardID); 
        }
        meshCardIDs.clear();
    }
}

void MeshRendererComponent::OnLoad()
{
    BeginLoadAssetBind()
    ResizeAssetArray(materials)
    LoadAssetBind(Model, model)
    LoadAssetArrayBind(Material, materials)
    EndLoadAssetBind
}

void MeshRendererComponent::OnSave()
{
    BeginSaveAssetBind()
    SaveAssetBind(model)
    SaveAssetArrayBind(materials)
    EndSaveAssetBind
}

void MeshRendererComponent::OnInit()
{
    Component::OnInit();

    materials.resize(model->GetSubmeshCount() + 1);  
    objectInfos.resize(model->GetSubmeshCount());
    while(objectIDs.size() < model->GetSubmeshCount())
    {
        objectIDs.push_back(EngineContext::RenderResource()->AllocateObjectID());
        meshCardIDs.push_back(EngineContext::RenderResource()->AllocateMeshCardID());
    }
}

void MeshRendererComponent::OnUpdate(float deltaTime)
{
    InitComponentIfNeed();

    std::shared_ptr<TransformComponent> transformComponent = TryGetComponent<TransformComponent>();
    if(!transformComponent) return;

    for(uint32_t i = 0; i < model->GetSubmeshCount(); i++)  // 逐子物体更新物体信息
    {
        objectInfos[i] = {
            .model = transformComponent->GetModelMat(),
            .prevModel = prevModel,
            .invModel = transformComponent->GetModelMatInv(),
            .animationID = 0,
            .materialID = materials[i] ? materials[i]->GetMaterialID() : 0,
            .vertexID = model->Submesh(i).vertexBuffer->vertexID,
            .indexID = model->Submesh(i).indexBuffer->indexID,
            .meshCardID = meshCardIDs[i],
            .sphere = model->Submesh(i).mesh->sphere,
            .box = model->Submesh(i).mesh->box,
            .debugData = Vec4::Zero()
        };

        EngineContext::RenderResource()->SetObjectInfo(objectInfos[i], objectIDs[i]);
    }
    
    prevModel = transformComponent->GetModelMat();
}

void MeshRendererComponent::SetModel(ModelRef model) 					
{ 
    this->model = model; 
    if(objectInfos.size() < model->GetSubmeshCount()) 
    {
        objectInfos.resize(model->GetSubmeshCount());
    }
    while(objectIDs.size() < model->GetSubmeshCount())
    {
        objectIDs.push_back(EngineContext::RenderResource()->AllocateObjectID());
        meshCardIDs.push_back(EngineContext::RenderResource()->AllocateMeshCardID());
    }
    materials.resize(model->GetSubmeshCount());  
}

void MeshRendererComponent::SetMaterial(MaterialRef material, uint32_t index)
{
    if(materials.size() <= index) this->materials.resize(index + 1);
    materials[index] = material;
}

void MeshRendererComponent::SetMaterials(std::vector<MaterialRef> materials, uint32_t firstIndex)
{
    if(this->materials.size() <= firstIndex + materials.size()) this->materials.resize(firstIndex + materials.size() + 1);
    for(uint32_t i = 0; i < materials.size(); i++)
    {
        this->materials[i + firstIndex] = materials[i];
    }
}

MaterialRef MeshRendererComponent::GetMaterial(uint32_t index)
{
    if(materials.size() > index) return materials[index];
    return nullptr;
}

void MeshRendererComponent::CollectDrawBatch(std::vector<DrawBatch>& batches)
{
    for(uint32_t i = 0; i < model->GetSubmeshCount(); i++)
    {   
        auto& submesh = model->Submesh(i);

        if(materials[i] != nullptr)
        {
            batches.push_back({
                .objectID = objectIDs[i],
                .vertexBuffer = submesh.vertexBuffer,
                .indexBuffer = submesh.indexBuffer,
                .clusterID = submesh.meshClusterID,
                .clusterGroupID = submesh.meshClusterGroupID,
                .material = materials[i]
            });
        }
    }
}

void MeshRendererComponent::CollectAccelerationStructureInstance(std::vector<RHIAccelerationStructureInstanceInfo>& instances)
{
    std::shared_ptr<TransformComponent> transformComponent = TryGetComponent<TransformComponent>();
    if(!transformComponent) return;

    for(uint32_t i = 0; i < model->GetSubmeshCount(); i++)
    {   
        auto& submesh = model->Submesh(i);

        if(materials[i] != nullptr)
        {
            RHIAccelerationStructureInstanceInfo info = {};
            info.instanceIndex = objectIDs[i];
            info.mask = 0xFF;
            info.shaderBindingTableOffset = 0;
            info.blas = submesh.blas;
            Math::Mat3x4(transformComponent->GetModelMat(), &info.transform[0][0]);

            instances.push_back(info);
        }
    }
}

void MeshRendererComponent::CollectSurfaceCacheTask(std::vector<SurfaceCacheTask>& tasks)
{
    std::shared_ptr<TransformComponent> transformComponent = TryGetComponent<TransformComponent>();
    if(!transformComponent) return;

    for(uint32_t i = 0; i < model->GetSubmeshCount(); i++)
    {   
        auto& submesh = model->Submesh(i);

        if(materials[i] != nullptr)
        {
            SurfaceCacheTask task = {};
            task.meshCardID = meshCardIDs[i];
            task.objectID = objectIDs[i];
            task.scale = transformComponent->GetScale();
            task.box = submesh.mesh->box;
            task.vertexBuffer = submesh.vertexBuffer;
            task.indexBuffer = submesh.indexBuffer;

            tasks.push_back(task);
        }
    }
}
