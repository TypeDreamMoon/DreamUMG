// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/DreamWidgetViewExtension.h"

#include "GlobalShader.h"
#include "PipelineStateCache.h"
#include "PostProcess/PostProcessMaterialInputs.h"
#include "RenderGraphUtils.h"
#include "RenderResource.h"
#include "RHIResourceUtils.h"
#include "RHIStaticStates.h"
#include "ScreenPass.h"
#include "SceneView.h"
#include "ShaderParameterStruct.h"
#include "UObject/ObjectKey.h"

namespace
{
	struct FDreamWidgetCompositeVertex
	{
		FVector3f Position = FVector3f::ZeroVector;
		FVector2f UV = FVector2f::ZeroVector;

		FDreamWidgetCompositeVertex() = default;

		FDreamWidgetCompositeVertex(const FVector3f& InPosition, const FVector2f& InUV)
			: Position(InPosition)
			, UV(InUV)
		{
		}
	};

	class FDreamWidgetCompositeVertexDeclaration : public FRenderResource
	{
	public:
		virtual void InitRHI(FRHICommandListBase& RHICmdList) override
		{
			FVertexDeclarationElementList Elements;
			const uint16 Stride = sizeof(FDreamWidgetCompositeVertex);
			Elements.Add(FVertexElement(0, STRUCT_OFFSET(FDreamWidgetCompositeVertex, Position), VET_Float3, 0, Stride));
			Elements.Add(FVertexElement(0, STRUCT_OFFSET(FDreamWidgetCompositeVertex, UV), VET_Float2, 1, Stride));
			VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
		}

		virtual void ReleaseRHI() override
		{
			VertexDeclarationRHI.SafeRelease();
		}

		FVertexDeclarationRHIRef VertexDeclarationRHI;
	};

	TGlobalResource<FDreamWidgetCompositeVertexDeclaration> GDreamWidgetCompositeVertexDeclaration;

	class FDreamWidgetCompositeVS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FDreamWidgetCompositeVS);
		SHADER_USE_PARAMETER_STRUCT(FDreamWidgetCompositeVS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}
	};

	class FDreamWidgetCompositePS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FDreamWidgetCompositePS);
		SHADER_USE_PARAMETER_STRUCT(FDreamWidgetCompositePS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
			SHADER_PARAMETER_TEXTURE(Texture2D, WidgetTexture)
			SHADER_PARAMETER_SAMPLER(SamplerState, WidgetTextureSampler)
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}
	};

	IMPLEMENT_GLOBAL_SHADER(FDreamWidgetCompositeVS, "/Plugin/DreamUMG/Private/DreamWidgetComposite.usf", "MainVS", SF_Vertex);
	IMPLEMENT_GLOBAL_SHADER(FDreamWidgetCompositePS, "/Plugin/DreamUMG/Private/DreamWidgetComposite.usf", "MainPS", SF_Pixel);

	BEGIN_SHADER_PARAMETER_STRUCT(FDreamWidgetPassParameters, )
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	struct FDreamWidgetProjectedMesh
	{
		FTextureRHIRef Texture;
		int32 SortPriority = 0;
		float DistanceSquared = 0.0f;
		TArray<FDreamWidgetCompositeVertex> Vertices;
		TArray<uint16> Indices;
	};

	FVector3f PixelToClipPosition(const FSceneView& View, const FVector2D& PixelLocation)
	{
		const FVector4 ScreenPosition = View.PixelToScreen(
			PixelLocation.X - View.UnscaledViewRect.Min.X,
			PixelLocation.Y - View.UnscaledViewRect.Min.Y,
			0.0f);
		return FVector3f(
			static_cast<float>(ScreenPosition.X),
			static_cast<float>(ScreenPosition.Y),
			0.0f);
	}

	bool ProjectWorldPosition(const FSceneView& View, const FVector& WorldPosition, FVector3f& OutClipPosition)
	{
		const FVector4 ScreenPosition = View.WorldToScreen(WorldPosition);
		if (ScreenPosition.W <= UE_KINDA_SMALL_NUMBER)
		{
			return false;
		}

		FVector2D PixelLocation;
		if (!View.WorldToPixel(WorldPosition, PixelLocation))
		{
			return false;
		}

		OutClipPosition = PixelToClipPosition(View, PixelLocation);
		return true;
	}

	bool IsBackFacing(const FDreamWidgetRenderItem& RenderItem, const FSceneView& View)
	{
		const FVector WidgetForward = RenderItem.Transform.TransformVectorNoScale(FVector::ForwardVector).GetSafeNormal();
		const FVector ToView = View.ViewMatrices.GetViewOrigin() - RenderItem.Transform.GetLocation();
		return FVector::DotProduct(WidgetForward, ToView) <= 0.0f;
	}

	bool IsProjectedBoundsVisible(const TArray<FDreamWidgetCompositeVertex>& Vertices)
	{
		float MinX = TNumericLimits<float>::Max();
		float MinY = TNumericLimits<float>::Max();
		float MaxX = TNumericLimits<float>::Lowest();
		float MaxY = TNumericLimits<float>::Lowest();

		for (const FDreamWidgetCompositeVertex& Vertex : Vertices)
		{
			MinX = FMath::Min(MinX, Vertex.Position.X);
			MinY = FMath::Min(MinY, Vertex.Position.Y);
			MaxX = FMath::Max(MaxX, Vertex.Position.X);
			MaxY = FMath::Max(MaxY, Vertex.Position.Y);
		}

		return !(MaxX < -1.0f || MinX > 1.0f || MaxY < -1.0f || MinY > 1.0f);
	}

	void AddPlaneMesh(const FDreamWidgetRenderItem& RenderItem, const FSceneView& View, FDreamWidgetProjectedMesh& OutMesh)
	{
		const float Width = RenderItem.Size.X;
		const float Height = RenderItem.Size.Y;
		const float Left = -Width * RenderItem.Pivot.X;
		const float Right = Width * (1.0f - RenderItem.Pivot.X);
		const float Bottom = -Height * RenderItem.Pivot.Y;
		const float Top = Height * (1.0f - RenderItem.Pivot.Y);

		const FVector LocalPositions[4] =
		{
			FVector(0.0f, Left, Bottom),
			FVector(0.0f, Right, Bottom),
			FVector(0.0f, Left, Top),
			FVector(0.0f, Right, Top)
		};

		const FVector2f UVs[4] =
		{
			FVector2f(0.0f, 1.0f),
			FVector2f(1.0f, 1.0f),
			FVector2f(0.0f, 0.0f),
			FVector2f(1.0f, 0.0f)
		};

		OutMesh.Vertices.Reserve(4);
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(LocalPositions); ++Index)
		{
			FVector3f ClipPosition;
			if (!ProjectWorldPosition(View, RenderItem.Transform.TransformPosition(LocalPositions[Index]), ClipPosition))
			{
				OutMesh.Vertices.Reset();
				OutMesh.Indices.Reset();
				return;
			}

			OutMesh.Vertices.Emplace(ClipPosition, UVs[Index]);
		}

		OutMesh.Indices = {0, 2, 3, 0, 3, 1};
	}

	void AddCylinderMesh(const FDreamWidgetRenderItem& RenderItem, const FSceneView& View, FDreamWidgetProjectedMesh& OutMesh)
	{
		const float ArcAngle = FMath::Max(FMath::DegreesToRadians(FMath::Abs(RenderItem.CylinderArcAngle)), 0.01f);
		const float ArcAngleSign = FMath::Sign(RenderItem.CylinderArcAngle);
		const int32 NumSegments = FMath::Clamp(FMath::RoundToInt(FMath::Lerp(4.0f, 32.0f, ArcAngle / PI)), 4, 32);
		const float Radius = RenderItem.Size.X / ArcAngle;
		const float Apothem = Radius * FMath::Cos(0.5f * ArcAngle);
		const float ChordLength = 2.0f * Radius * FMath::Sin(0.5f * ArcAngle);
		const float HalfChordLength = ChordLength * 0.5f;
		const float PivotOffsetY = ChordLength * (0.5f - RenderItem.Pivot.X);
		const float Bottom = -RenderItem.Size.Y * RenderItem.Pivot.Y;
		const float Top = RenderItem.Size.Y * (1.0f - RenderItem.Pivot.Y);
		const float RadiansPerStep = ArcAngle / NumSegments;
		float Angle = -ArcAngle * 0.5f;

		OutMesh.Vertices.Reserve((NumSegments + 1) * 2);
		OutMesh.Indices.Reserve(NumSegments * 6);

		for (int32 SegmentIndex = 0; SegmentIndex <= NumSegments; ++SegmentIndex)
		{
			const float UVX = static_cast<float>(SegmentIndex) / static_cast<float>(NumSegments);
			const FVector BaseLocalPosition(
				ArcAngleSign * (Radius * FMath::Cos(Angle) - Apothem),
				Radius * FMath::Sin(Angle) + PivotOffsetY,
				Bottom);

			for (int32 VerticalIndex = 0; VerticalIndex < 2; ++VerticalIndex)
			{
				const FVector LocalPosition = VerticalIndex == 0
					? BaseLocalPosition
					: FVector(BaseLocalPosition.X, BaseLocalPosition.Y, Top);

				FVector3f ClipPosition;
				if (!ProjectWorldPosition(View, RenderItem.Transform.TransformPosition(LocalPosition), ClipPosition))
				{
					OutMesh.Vertices.Reset();
					OutMesh.Indices.Reset();
					return;
				}

				OutMesh.Vertices.Emplace(
					ClipPosition,
					FVector2f(UVX, VerticalIndex == 0 ? 1.0f : 0.0f));
			}

			Angle += RadiansPerStep;
		}

		for (int32 SegmentIndex = 0; SegmentIndex < NumSegments; ++SegmentIndex)
		{
			const uint16 BaseIndex = static_cast<uint16>(SegmentIndex * 2);
			OutMesh.Indices.Add(BaseIndex + 0);
			OutMesh.Indices.Add(BaseIndex + 3);
			OutMesh.Indices.Add(BaseIndex + 1);
			OutMesh.Indices.Add(BaseIndex + 0);
			OutMesh.Indices.Add(BaseIndex + 2);
			OutMesh.Indices.Add(BaseIndex + 3);
		}
	}

	bool BuildProjectedMesh(const FDreamWidgetRenderItem& RenderItem, const FSceneView& View, FDreamWidgetProjectedMesh& OutMesh)
	{
		if (!RenderItem.bVisible || !RenderItem.Texture.IsValid() || RenderItem.Size.X <= 0.0f || RenderItem.Size.Y <= 0.0f)
		{
			return false;
		}

		if (!RenderItem.bTwoSided && IsBackFacing(RenderItem, View))
		{
			return false;
		}

		OutMesh.Texture = RenderItem.Texture;
		OutMesh.SortPriority = RenderItem.SortPriority;
		OutMesh.DistanceSquared = FVector::DistSquared(RenderItem.Transform.GetLocation(), View.ViewMatrices.GetViewOrigin());

		switch (RenderItem.GeometryMode)
		{
		case EWidgetGeometryMode::Cylinder:
			AddCylinderMesh(RenderItem, View, OutMesh);
			break;

		case EWidgetGeometryMode::Plane:
		default:
			AddPlaneMesh(RenderItem, View, OutMesh);
			break;
		}

		return OutMesh.Vertices.Num() > 0 && OutMesh.Indices.Num() > 0 && IsProjectedBoundsVisible(OutMesh.Vertices);
	}

	void AddCompositePass(
		FRDGBuilder& GraphBuilder,
		FRDGTextureRef OutputTexture,
		const FIntRect& ViewRect,
		ERenderTargetLoadAction LoadAction,
		FGlobalShaderMap* GlobalShaderMap,
		TArray<FDreamWidgetProjectedMesh>&& ProjectedMeshes)
	{
		if (OutputTexture == nullptr || ProjectedMeshes.Num() == 0 || GlobalShaderMap == nullptr)
		{
			return;
		}

		FDreamWidgetPassParameters* PassParameters = GraphBuilder.AllocParameters<FDreamWidgetPassParameters>();
		PassParameters->RenderTargets[0] = FRenderTargetBinding(OutputTexture, LoadAction);

		GraphBuilder.AddPass(
			RDG_EVENT_NAME("DreamWidgetComposite"),
			PassParameters,
			ERDGPassFlags::Raster,
			[ProjectedMeshes = MoveTemp(ProjectedMeshes), GlobalShaderMap, ViewRect, NumSamples = OutputTexture->Desc.NumSamples](FRHICommandListImmediate& RHICmdList)
			{
				TShaderMapRef<FDreamWidgetCompositeVS> VertexShader(GlobalShaderMap);
				TShaderMapRef<FDreamWidgetCompositePS> PixelShader(GlobalShaderMap);

				for (const FDreamWidgetProjectedMesh& Mesh : ProjectedMeshes)
				{
					if (!Mesh.Texture.IsValid() || Mesh.Vertices.Num() == 0 || Mesh.Indices.Num() == 0)
					{
						continue;
					}

					RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

					FGraphicsPipelineStateInitializer GraphicsPSOInit;
					RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
					GraphicsPSOInit.BlendState = TStaticBlendState<
						CW_RGBA,
						BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha,
						BO_Add, BF_One, BF_InverseSourceAlpha>::GetRHI();
					GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
					GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
					GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GDreamWidgetCompositeVertexDeclaration.VertexDeclarationRHI;
					GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
					GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
					GraphicsPSOInit.PrimitiveType = PT_TriangleList;
					GraphicsPSOInit.NumSamples = NumSamples;
					SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);

					FDreamWidgetCompositeVS::FParameters VertexParameters;
					SetShaderParameters(RHICmdList, VertexShader, VertexShader.GetVertexShader(), VertexParameters);

					FDreamWidgetCompositePS::FParameters PixelParameters;
					PixelParameters.WidgetTexture = Mesh.Texture;
					PixelParameters.WidgetTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
					SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), PixelParameters);

					FBufferRHIRef VertexBufferRHI = UE::RHIResourceUtils::CreateVertexBufferFromArray(
						RHICmdList,
						TEXT("DreamWidgetCompositeVertexBuffer"),
						EBufferUsageFlags::Volatile,
						MakeConstArrayView(Mesh.Vertices));
					FBufferRHIRef IndexBufferRHI = UE::RHIResourceUtils::CreateIndexBufferFromArray(
						RHICmdList,
						TEXT("DreamWidgetCompositeIndexBuffer"),
						EBufferUsageFlags::Volatile,
						MakeConstArrayView(Mesh.Indices));

					RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
					RHICmdList.DrawIndexedPrimitive(IndexBufferRHI, 0, 0, Mesh.Vertices.Num(), 0, Mesh.Indices.Num() / 3, 1);
				}
			});
	}
}

FDreamWidgetViewExtension::FDreamWidgetViewExtension(const FAutoRegister& AutoRegister, UWorld* InWorld)
	: FSceneViewExtensionBase(AutoRegister)
	, World(InWorld)
{
}

void FDreamWidgetViewExtension::SubscribeToPostProcessingPass(
	EPostProcessingPass Pass,
	const FSceneView& InView,
	FPostProcessingPassDelegateArray& InOutPassCallbacks,
	bool bIsPassEnabled)
{
	if (!bIsPassEnabled || !HasRenderItems())
	{
		return;
	}

	EPostProcessingPass DesiredPass = EPostProcessingPass::Tonemap;
	if (InView.AntiAliasingMethod == AAM_FXAA)
	{
		DesiredPass = EPostProcessingPass::FXAA;
	}
	else if (InView.AntiAliasingMethod == AAM_SMAA)
	{
		DesiredPass = EPostProcessingPass::SMAA;
	}

	if (Pass == DesiredPass)
	{
		InOutPassCallbacks.Add(FAfterPassCallbackDelegate::CreateRaw(this, &FDreamWidgetViewExtension::PostProcessPassAfterRender_RenderThread));
	}
}

FScreenPassTexture FDreamWidgetViewExtension::PostProcessPassAfterRender_RenderThread(
	FRDGBuilder& GraphBuilder,
	const FSceneView& View,
	const FPostProcessMaterialInputs& Inputs)
{
	FScreenPassTextureSlice SceneColorSlice = Inputs.GetInput(EPostProcessMaterialInput::SceneColor);
	if (!SceneColorSlice.IsValid())
	{
		return FScreenPassTexture();
	}

	FScreenPassTexture SceneColor(SceneColorSlice);

	TArray<FDreamWidgetRenderItem> RenderItemsCopy;
	{
		FScopeLock ScopeLock(&RenderItemsCS);
		RenderItems.GenerateValueArray(RenderItemsCopy);
	}

	if (RenderItemsCopy.Num() == 0)
	{
		return SceneColor;
	}

	TArray<FDreamWidgetProjectedMesh> ProjectedMeshes;
	ProjectedMeshes.Reserve(RenderItemsCopy.Num());

	for (const FDreamWidgetRenderItem& RenderItem : RenderItemsCopy)
	{
		FDreamWidgetProjectedMesh ProjectedMesh;
		if (BuildProjectedMesh(RenderItem, View, ProjectedMesh))
		{
			ProjectedMeshes.Add(MoveTemp(ProjectedMesh));
		}
	}

	if (ProjectedMeshes.Num() == 0)
	{
		return SceneColor;
	}

	ProjectedMeshes.Sort([](const FDreamWidgetProjectedMesh& A, const FDreamWidgetProjectedMesh& B)
	{
		if (A.SortPriority != B.SortPriority)
		{
			return A.SortPriority < B.SortPriority;
		}

		return A.DistanceSquared > B.DistanceSquared;
	});

	FScreenPassRenderTarget Output = Inputs.OverrideOutput;
	if (!Output.IsValid())
	{
		Output = FScreenPassRenderTarget(SceneColor.Texture, SceneColor.ViewRect, View.GetOverwriteLoadAction());
	}

	AddCompositePass(GraphBuilder, Output.Texture, Output.ViewRect, ERenderTargetLoadAction::ELoad, GetGlobalShaderMap(View.GetFeatureLevel()), MoveTemp(ProjectedMeshes));
	return MoveTemp(Output);
}

int32 FDreamWidgetViewExtension::GetPriority() const
{
	return 100;
}

bool FDreamWidgetViewExtension::IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const
{
	return HasRenderItems();
}

void FDreamWidgetViewExtension::UpsertRenderItem(const FDreamWidgetRenderItem& InRenderItem)
{
	FScopeLock ScopeLock(&RenderItemsCS);
	RenderItems.Add(InRenderItem.ComponentId, InRenderItem);
}

void FDreamWidgetViewExtension::RemoveRenderItem(uint32 ComponentId)
{
	FScopeLock ScopeLock(&RenderItemsCS);
	RenderItems.Remove(ComponentId);
}

bool FDreamWidgetViewExtension::HasRenderItems() const
{
	FScopeLock ScopeLock(&RenderItemsCS);
	return RenderItems.Num() > 0;
}

namespace DreamWidgetViewExtensionRegistry
{
	namespace
	{
		FCriticalSection RegistryCS;
		TMap<TObjectKey<UWorld>, TWeakPtr<FDreamWidgetViewExtension, ESPMode::ThreadSafe>> Extensions;
	}

	TSharedPtr<FDreamWidgetViewExtension, ESPMode::ThreadSafe> Find(UWorld* World)
	{
		if (World == nullptr)
		{
			return nullptr;
		}

		FScopeLock ScopeLock(&RegistryCS);
		if (const TWeakPtr<FDreamWidgetViewExtension, ESPMode::ThreadSafe>* Existing = Extensions.Find(TObjectKey<UWorld>(World)))
		{
			return Existing->Pin();
		}

		return nullptr;
	}

	TSharedPtr<FDreamWidgetViewExtension, ESPMode::ThreadSafe> Get(UWorld* World)
	{
		if (World == nullptr)
		{
			return nullptr;
		}

		FScopeLock ScopeLock(&RegistryCS);
		const TObjectKey<UWorld> WorldKey(World);
		if (TWeakPtr<FDreamWidgetViewExtension, ESPMode::ThreadSafe>* Existing = Extensions.Find(WorldKey))
		{
			if (TSharedPtr<FDreamWidgetViewExtension, ESPMode::ThreadSafe> ExistingPinned = Existing->Pin())
			{
				return ExistingPinned;
			}
		}

		TSharedRef<FDreamWidgetViewExtension, ESPMode::ThreadSafe> NewExtension = FSceneViewExtensions::NewExtension<FDreamWidgetViewExtension>(World);
		Extensions.Add(WorldKey, NewExtension);
		return NewExtension;
	}
}
