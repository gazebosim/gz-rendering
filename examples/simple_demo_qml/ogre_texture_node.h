#ifndef OGRE_TEXTURE_NODE_HH
#define OGRE_TEXTURE_NODE_HH

#include <QSGTextureProvider>
#include <QSGSimpleTextureNode>

class OgreTextureNode : public QSGTextureProvider, public QSGSimpleTextureNode
{
Q_OBJECT

 public:
  explicit OgreTextureNode(QQuickItem *item = nullptr);
  ~OgreTextureNode() override;
  [[nodiscard]] QSGTexture *texture() const override;
  void sync();

 private slots:
  void render();

 private:
  void createNativeTexture();
  void releaseNativeTexture();
  void initialize();

  QQuickItem *m_item;
  QQuickWindow *m_window;
  QSize m_pixelSize;
  qreal m_dpr;

  bool m_initialized = false;

  QVulkanInstance *m_inst = nullptr;
  VkPhysicalDevice m_physDev = VK_NULL_HANDLE;
  VkDevice m_dev = VK_NULL_HANDLE;
  QVulkanDeviceFunctions *m_devFuncs = nullptr;
  QVulkanFunctions *m_funcs = nullptr;

  VkImage m_output = VK_NULL_HANDLE;
  VkImageLayout m_outputLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkDeviceMemory m_outputMemory = VK_NULL_HANDLE;
  VkImageView m_outputView = VK_NULL_HANDLE;
  QSGTexture *m_sgWrapperTexture = nullptr;
};

#endif  // OGRE_TEXTURE_NODE_HH
