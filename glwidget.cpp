#include "glwidget.h"
#include <QMouseEvent>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDir>
#include <QImage>
#include <QWheelEvent>

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
}

GLWidget::~GLWidget()
{
    makeCurrent();
    delete m_texture;
    doneCurrent();
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);

    QString objFile = "test.obj";
    QString mtlFile = "test.mtl";

    if (!loadObj(objFile))
        qWarning() << "Failed to load OBJ file";

    if (!loadMtl(mtlFile))
        qWarning() << "Failed to load MTL file";
}

void GLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    m_proj.setToIdentity();
    m_proj.perspective(45.0f, float(w) / h, 0.1f, 100.0f);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // --- 绘制渐变背景 ---
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glBegin(GL_QUADS);
    glColor3f(0.7f, 0.7f, 0.7f); // 上方较深灰
    glVertex2f(0.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glColor3f(0.9f, 0.9f, 0.9f); // 下方浅灰
    glVertex2f(1.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glEnd();
    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3f(1.0f, 1.0f, 1.0f); // 恢复默认颜色
    // --- 渐变背景结束 ---

    // --- 原来的 3D 模型绘制 ---
    QMatrix4x4 model;
    model.translate(0, 0, -5);
    model.scale(m_scale);
    model.rotate(m_angleX, 1, 0, 0);
    model.rotate(m_angleY, 0, 1, 0);

    QMatrix4x4 mvp = m_proj * model;
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(mvp.constData());

    if (m_texture)
        m_texture->bind();

    glBegin(GL_TRIANGLES);
    for (const auto &v : m_vertices) {
        glNormal3f(v.normal.x(), v.normal.y(), v.normal.z());
        glTexCoord2f(v.texCoord.x(), v.texCoord.y());
        glVertex3f(v.position.x(), v.position.y(), v.position.z());
    }
    glEnd();

    if (m_texture)
        m_texture->release();
}



void GLWidget::mousePressEvent(QMouseEvent *e)
{
    m_lastPos = e->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *e)
{
    int dx = e->x() - m_lastPos.x();
    int dy = e->y() - m_lastPos.y();

    if (e->buttons() & Qt::LeftButton) {
        m_angleX += dy;
        m_angleY += dx;
        update();
    }

    m_lastPos = e->pos();
}

bool GLWidget::loadObj(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open OBJ:" << filename;
        return false;
    }

    QVector<QVector3D> positions;
    QVector<QVector3D> normals;
    QVector<QVector2D> texcoords;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;

        QStringList parts = line.split(" ", Qt::SkipEmptyParts);
        if (parts[0] == "v" && parts.size() >= 4)
            positions.append(QVector3D(parts[1].toFloat(), parts[2].toFloat(), parts[3].toFloat()));
        else if (parts[0] == "vn" && parts.size() >= 4)
            normals.append(QVector3D(parts[1].toFloat(), parts[2].toFloat(), parts[3].toFloat()));
        else if (parts[0] == "vt" && parts.size() >= 3)
            // texcoords.append(QVector2D(parts[1].toFloat(), 1.0f - parts[2].toFloat())); // flip Y
            texcoords.append(QVector2D(parts[1].toFloat(), parts[2].toFloat())); // 不翻转
        else if (parts[0] == "f" && parts.size() >= 4) {
            for (int i = 1; i <= 3; ++i) {
                QStringList idx = parts[i].split('/');
                int vi = idx.value(0).toInt() - 1;
                int ti = idx.size() > 1 && !idx[1].isEmpty() ? idx[1].toInt() - 1 : -1;
                int ni = idx.size() > 2 ? idx[2].toInt() - 1 : -1;

                Vertex v;
                v.position = (vi >= 0 && vi < positions.size()) ? positions[vi] : QVector3D();
                v.normal = (ni >= 0 && ni < normals.size()) ? normals[ni] : QVector3D(0, 1, 0);
                v.texCoord = (ti >= 0 && ti < texcoords.size()) ? texcoords[ti] : QVector2D(0, 0);
                m_vertices.append(v);
            }
        }
    }

    qDebug() << "Loaded OBJ:" << filename
             << "V:" << positions.size()
             << "VT:" << texcoords.size()
             << "VN:" << normals.size()
             << "F:" << m_vertices.size()/3;

    return !m_vertices.isEmpty();
}

bool GLWidget::loadMtl(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open MTL:" << filename;
        return false;
    }

    QString texturePath;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith("map_Kd")) {
            QStringList parts = line.split(" ", Qt::SkipEmptyParts);
            if (parts.size() >= 2)
                texturePath = parts[1];
        }
    }

    if (!texturePath.isEmpty()) {
        QFileInfo fi(filename);
        QString fullPath = fi.dir().filePath(texturePath);
        qDebug() << "Loading texture:" << fullPath;
        m_texture = new QOpenGLTexture(QImage(fullPath).mirrored());
        return true;
    }

    return false;
}

void GLWidget::wheelEvent(QWheelEvent *e)
{
    if (e->angleDelta().y() > 0)
        m_scale *= 1.1f;  // 放大
    else
        m_scale /= 1.1f;  // 缩小

    if (m_scale < 0.1f) m_scale = 0.1f;
    if (m_scale > 10.0f) m_scale = 10.0f;

    update();
}
