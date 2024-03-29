#pragma once

#include "be_frameInfo.hpp"
#include "be_gameObject.hpp"
#include "be_pipeline.hpp"
#include "be_vulkanApp.hpp"

namespace be{

/**
 * Forward declaration of the IRenderSubSystem interface
 * @see IRenderSubSystem
*/
class IRenderSubSystem;

/**
 * Smart pointer to a render sub sytstem
*/
using IRenderSubSystemPtr = std::shared_ptr<IRenderSubSystem>;

/**
 * @brief An interface to represent a render sub system
 * This interface should be implemented for every type of rendering the user want (color pass through, lambert, ...)
*/
class IRenderSubSystem{

    protected:
        /**
         * A smart pointer to a vulkan application
         * @see VulkanApp
        */
        VulkanAppPtr _VulkanApp = nullptr;

        /**
         * A smart pointer to a graphics pipeline
         * @see Pipeline
        */
        PipelinePtr _Pipeline = nullptr;

        /**
         * The pipline layout associated to the current pipeline
        */
        VkPipelineLayout _PipelineLayout = nullptr;

        /**
         * A list of descriptor sets
        */
        std::vector<VkDescriptorSet> _DescriptorSets{};

        /**
         * Boolean set to true if the sub system needs to bind new descriptor sets per objects
        */
        bool _NeedPerObjectBind = false;

    public:
        /**
         * A basic constructor
         * @param vulkanApp A pointer to a vulkanApp
         * @param renderPass The render pass used to create the graphics pipeline
        */
        IRenderSubSystem(VulkanAppPtr vulkanApp, VkRenderPass renderPass);

        /**
         * A virtual destructor to make the class purely virtual
        */
        virtual ~IRenderSubSystem() = default;

        /**
         * A function to render game objects
         * @param framInfo The current frame
         * @see FrameInfo
        */
        virtual void renderGameObjects(FrameInfo& frameInfo) = 0;

        /**
         * A clean up function
        */
        virtual void cleanUp();

        /**
         * The function representing how an object should be rendererd
         * @param object The object to render
         * @see GameObject
         * @note This is the most important function of the interface, it describes how and when an object should be rendered
        */
        virtual void renderingFunction(GameObject object) = 0;

        /**
         * Getter to see if the sub system need to be bind for each object
         * @return The value of _NeedPerObjectBind
        */
        bool needPerObjectBind() const;
        

    public:
        /**
         * A getter to the current pipeline
         * @return The associated pipeline
        */
        PipelinePtr getPipeline() const;

        /**
         * A getter to the pipeline layout
         * @return The associated pipeline layout
        */
        VkPipelineLayout getPipelineLayout() const;

        /**
         * A getter to the descriptor sets
         * @return The associated descriptor sets
        */
        std::vector<VkDescriptorSet> getDescriptorSets() const;

        /**
         * Create a command to bind the descriptor sets
         * @param frameInfo the current frame
         * @param bindPoint the pipeline bind point (default to GRAPHICS)
        */
        void cmdBindDescriptoSets(FrameInfo& frameInfo, 
            VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const;

        /**
         * Update the descriptor sets
         * @param object The current game object
         * @param frameInfo the current frame
         * @note This function should only be called when _NeedPerObjectBind is set to true
        */
        virtual void updateDescriptorSets(GameObject object, FrameInfo& frameInfo);


    protected:
        /**
         * An initializer for the pipeline layout
        */
        virtual void initPipelineLayout() = 0;

        /**
         * An initializer for the pipeline
         * @param renderPass The render pass associated to the pipeline
        */
        virtual void initPipeline(VkRenderPass renderPass) = 0;

        /**
         * A cleaner for the pipeline layout
        */
        virtual void cleanUpPipelineLayout() = 0;

        /**
         * A cleaner for the pipeline
        */
        virtual void cleanUpPipeline();
};

};