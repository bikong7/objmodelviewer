#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QMatrix4x4>
#include <QVector2D>
#include <QVector3D>
#include <QPoint>

struct Vertex {
    QVector3D position;
    QVector3D normal;
    QVector2D texCoord;
};

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit GLWidget(QWidget *parent = nullptr);
    ~GLWidget();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;  // 👈 新增滚轮事件

private:
    bool loadObj(const QString &filename);
    bool loadMtl(const QString &filename);

    QVector<Vertex> m_vertices;
    QOpenGLTexture *m_texture = nullptr;

    float m_angleX = 0.0f;
    float m_angleY = 0.0f;
    float m_scale = 1.0f;  // 👈 新增缩放因子
    QPoint m_lastPos;
    QMatrix4x4 m_proj;
};
