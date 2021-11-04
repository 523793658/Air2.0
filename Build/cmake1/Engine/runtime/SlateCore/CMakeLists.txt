SET(LIB_NAME SlateCore)
SET(Widgets_Source
     ${SlateCore_SOURCE_DIR}/Widgets/SWindow.h  
     ${SlateCore_SOURCE_DIR}/Widgets/SWindow.cpp  
     ${SlateCore_SOURCE_DIR}/Widgets/SCompoundWidget.h  
     ${SlateCore_SOURCE_DIR}/Widgets/SWidget.h  
     ${SlateCore_SOURCE_DIR}/Widgets/SWidget.cpp  
     ${SlateCore_SOURCE_DIR}/Widgets/DeclarativeSyntaxSupport.h  
     ${SlateCore_SOURCE_DIR}/Widgets/SUserWidget.h  
     ${SlateCore_SOURCE_DIR}/Widgets/SUserWidget.cpp  
     ${SlateCore_SOURCE_DIR}/Widgets/SNullWidget.h  
     ${SlateCore_SOURCE_DIR}/Widgets/SOverlay.h  
     ${SlateCore_SOURCE_DIR}/Widgets/SOverlay.cpp  
     ${SlateCore_SOURCE_DIR}/Widgets/SPanel.cpp  
     ${SlateCore_SOURCE_DIR}/Widgets/SPanel.h  
    )
SOURCE_GROUP(Widgets FILES ${Widgets_Source})
    
SET(Application_Source
     ${SlateCore_SOURCE_DIR}/Application/SlateApplicationBase.h  
     ${SlateCore_SOURCE_DIR}/Application/SlateApplicationBase.cpp  
     ${SlateCore_SOURCE_DIR}/Application/SlateWondowHelper.h  
     ${SlateCore_SOURCE_DIR}/Application/SlateWondowHelper.cpp  
    )
SOURCE_GROUP(Application ${Application_Source})

SET(SlateCore_Source
     ${SlateCore_SOURCE_DIR}/SlateCore.h  
    )
    
SET(Input_Source 
     ${SlateCore_SOURCE_DIR}/Input/Events.h  
     ${SlateCore_SOURCE_DIR}/Input/Events.cpp  
     ${SlateCore_SOURCE_DIR}/Input/Reply.h  
     ${SlateCore_SOURCE_DIR}/Input/Reply.cpp  
     ${SlateCore_SOURCE_DIR}/Input/ReplyBase.h  
     ${SlateCore_SOURCE_DIR}/Input/ReplyBase.cpp  
    )
SOURCE_GROUP(Input FILES ${Input_Source})
    
    
SET(Rendering_Source
     ${SlateCore_SOURCE_DIR}/Rendering/SlateRenderer.h  
     ${SlateCore_SOURCE_DIR}/Rendering/SlateRenderer.cpp  
     ${SlateCore_SOURCE_DIR}/Rendering/RenderingCommon.h  
     ${SlateCore_SOURCE_DIR}/Rendering/RenderingCommon.cpp  
     ${SlateCore_SOURCE_DIR}/Rendering/SlateDrawBuffer.h  
     ${SlateCore_SOURCE_DIR}/Rendering/SlateDrawBuffer.cpp  
     ${SlateCore_SOURCE_DIR}/Rendering/DrawElements.h  
     ${SlateCore_SOURCE_DIR}/Rendering/DrawElements.cpp  
     ${SlateCore_SOURCE_DIR}/Rendering/ElementBatcher.h  
     ${SlateCore_SOURCE_DIR}/Rendering/ElementBatcher.cpp  
     ${SlateCore_SOURCE_DIR}/Rendering/RenderingPolicy.h  
     ${SlateCore_SOURCE_DIR}/Rendering/SlateLayoutTransform.h  
     ${SlateCore_SOURCE_DIR}/Rendering/SlateLayoutTransform.cpp  
     ${SlateCore_SOURCE_DIR}/Rendering/SlateRenderTransform.h  
    )
SOURCE_GROUP(Rendering FILES ${Rendering_Source})
    
SET(Textures_Source
     ${SlateCore_SOURCE_DIR}/Textures/SlateShaderResource.h  
     ${SlateCore_SOURCE_DIR}/Textures/SlateShaderResource.cpp  
    )
SOURCE_GROUP(Textures FILES Textures_Source)
    
    
SET(Layout_Source 
     ${SlateCore_SOURCE_DIR}/Layout/Margin.h  
     ${SlateCore_SOURCE_DIR}/Layout/Margin.cpp  
     ${SlateCore_SOURCE_DIR}/Layout/WidgetPath.h  
     ${SlateCore_SOURCE_DIR}/Layout/WidgetPath.cpp  
     ${SlateCore_SOURCE_DIR}/Layout/SlateRect.cpp  
     ${SlateCore_SOURCE_DIR}/Layout/SlateRect.h  
     ${SlateCore_SOURCE_DIR}/Layout/ArrangedChildren.h  
     ${SlateCore_SOURCE_DIR}/Layout/ArrangedChildren.cpp  
     ${SlateCore_SOURCE_DIR}/Layout/Geometry.h  
     ${SlateCore_SOURCE_DIR}/Layout/Geometry.cpp  
    )
SOURCE_GROUP(Layout FILES ${Layout_Source})
    
SET(Type_Source
     ${SlateCore_SOURCE_DIR}/Type/SlateEnums.h  
)
SOURCE_GROUP(Type FILES ${Type_Source})

ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${Type_Source}
    ${Layout_Source}
    ${Textures_Source}
    ${Rendering_Source}
    ${Input_Source}
    ${SlateCore_Source}
    ${Application_Source}
    ${Widgets_Source}
)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/Runtime)
    
target_link_libraries(${LIB_NAME} 
    core
    InputCore
    )
    
    
INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)