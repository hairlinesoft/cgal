// Author     : Nader Salman

#ifndef _Gyroviz_cdt2_
#define _Gyroviz_cdt2_

#include <algorithm>

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/origin.h>

#include "Gyroviz_info_for_cdt2.h"

#include <CImg.h>
using namespace cimg_library;

// Tds vertex class must inherit from Triangulation_vertex_base_with_info_2<CGAL::Point_3,K>
template < class Gt, class Tds >
class Gyroviz_cdt2 : public CGAL::Constrained_Delaunay_triangulation_2<Gt, Tds, Itag>
{
  // Private types
private:

  typedef CGAL::Constrained_Delaunay_triangulation_2<Gt, Tds, Itag>  Base;

  // Public types
public:

  // Repeat Constrained_Delaunay_triangulation_2 public types
  typedef Tds Triangulation_data_structure;
  typedef CGAL::Exact_predicates_tag                 Itag;
  typedef Gt  Geom_traits;
  typedef typename Geom_traits::FT FT;
  typedef typename Geom_traits::Point_2              Point_2;
  typedef typename Geom_traits::Point_3              Point_3;
  typedef typename Geom_traits::Vector_2             Vector_2;
  typedef typename Geom_traits::Vector_3             Vector_3;
  typedef typename Geom_traits::Sphere_3             Sphere;
  typedef typename Geom_traits::Iso_cuboid_3         Iso_cuboid_3;
  typedef typename Base::Face_handle                 Face_handle;
  typedef typename Base::Vertex_handle               Vertex_handle;
  typedef typename Base::Edge                        Edge;
  typedef typename Base::Edge_circulator             Edge_circulator;
  typedef typename Base::Finite_edges_iterator       Finite_edges_iterator;
  typedef typename Base::Finite_faces_iterator       Finite_faces_iterator;
  typedef typename Base::Finite_vertices_iterator    Finite_vertices_iterator;

  // Data members
private:

  Iso_cuboid_3 m_bounding_box; // Triangulation's bounding box
  Point_3 m_barycenter; // Triangulation's barycenter
  FT m_standard_deviation; // Triangulation's standard deviation

  // Public methods
public:

  // Default constructor, copy constructor and operator =() are fine

  bool save_pnt(char *pFilename)
  {
    // TODO
  }

  bool read_pnt(char *pFilename)
  {
    FILE *pFile = fopen(pFilename,"r");
    if(pFile == NULL)
      return false;

    //scan vertices and add them to triangulation with corresponding info
    int lineNumber = 0;
    char pLine[512];

    while( fgets(pLine, 512, pFile))
    {
      lineNumber++;

      // read 2D/3D coordinates 
      //(on suppose avoir fait du traitement 
      // des fichiers pnt a l'etape precedente)
      int  unused1, is_reconstructed;
      double p,q,x,y,z;

      if (sscanf(pLine,"%lf\t%lf\t%d\t%d\t%lf\t%lf\t%lf", &p,&q,&unused1,&is_reconstructed,&x,&y,&z) == 7)
      {
        if (is_reconstructed == 2)
        {
          Point_2 point_2(p,q);
          Point_3 point_3(x,y,z);

          Vertex_handle vh = this->insert(point_2);
          vh->info() = Gyroviz_info_for_cdt2(point_3,false);
        }
      }
    }
    fclose(pFile);
    update_bounding_box();
    return (this->number_of_vertices() > 0);
  }


  // image is segmented 
  void add_on_border_2D_vertices(CImg <unsigned char> image)
  {
    Finite_vertices_iterator fv = this->finite_vertices_begin();
    for(; fv != this->finite_vertices_end(); ++fv)
    {
      // if pixel any of 9x9 surrounding pixels is on border keep vertex(white color is used in frei-chen gradient operator)


      if (image((unsigned int)fv->point().x(),(unsigned int)fv->point().y(),0,0)==255)
      {
        fv->info().set_flag(true);
      }

      else if((unsigned int)fv->point().x() == 0 && (unsigned int)fv->point().y() == 0) //upper left pixel
      {
        if (image((unsigned int)fv->point().x()+1,(unsigned int)fv->point().y(),0,0)  == 255 ||
          image((unsigned int)fv->point().x(),(unsigned int)fv->point().y()+1,0,0)  == 255 ||
          image((unsigned int)fv->point().x()+1,(unsigned int)fv->point().y()+1,0,0)== 255)
        {  
          fv->info().set_flag(true);
        }

      }
      else if((unsigned int)fv->point().x() == image.dimx() && (unsigned int)fv->point().y() == 0) //upper right pixel
      {
        if (image((unsigned int)fv->point().x()-1,(unsigned int)fv->point().y(),0,0)  == 255 ||
          image((unsigned int)fv->point().x(),(unsigned int)fv->point().y()+1,0,0)  == 255 ||
          image((unsigned int)fv->point().x()-1,(unsigned int)fv->point().y()+1,0,0)== 255) 
        {
          fv->info().set_flag(true);
        }
      }

      else if((unsigned int)fv->point().x() == 0 && (unsigned int)fv->point().y() == image.dimy()) //lower left pixel
      {
        if (image((unsigned int)fv->point().x(),(unsigned int)fv->point().y()-1,0,0)  == 255 ||
          image((unsigned int)fv->point().x()+1,(unsigned int)fv->point().y(),0,0)  == 255 ||
          image((unsigned int)fv->point().x()+1,(unsigned int)fv->point().y()-1,0,0)== 255) 
        {
          fv->info().set_flag(true);
        }

      }

      else if((unsigned int)fv->point().x() == image.dimx() && (unsigned int)fv->point().y() == image.dimy()) //lower right pixel
      {
        if (image((unsigned int)fv->point().x(),(unsigned int)fv->point().y()-1,0,0)  == 255 ||
          image((unsigned int)fv->point().x()-1,(unsigned int)fv->point().y(),0,0)  == 255 ||
          image((unsigned int)fv->point().x()-1,(unsigned int)fv->point().y()-1,0,0)== 255) 
        {
          fv->info().set_flag(true);
        }
      }

      else if((unsigned int)fv->point().x() > 0 && (unsigned int)fv->point().x() < image.dimx() && (unsigned int)fv->point().y() == 0) // upper band
      {
        if (image((unsigned int)fv->point().x()-1,(unsigned int)fv->point().y(),0,0)  == 255 ||
          image((unsigned int)fv->point().x()-1,(unsigned int)fv->point().y()+1,0,0)== 255 ||
          image((unsigned int)fv->point().x(),  (unsigned int)fv->point().y()+1,0,0)== 255 ||
          image((unsigned int)fv->point().x()+1,(unsigned int)fv->point().y(),0,0)  == 255 ||
          image((unsigned int)fv->point().x()+1,(unsigned int)fv->point().y()+1,0,0)== 255) 
        {
          fv->info().set_flag(true);
        }      
      }

      else if((unsigned int)fv->point().x() > 0 && (unsigned int)fv->point().x() < image.dimx() && (unsigned int)fv->point().y() == image.dimy()) // lower band
      {
        if (image((unsigned int)fv->point().x()-1,(unsigned int)fv->point().y(),0,0)  == 255 ||
          image((unsigned int)fv->point().x()-1,(unsigned int)fv->point().y()-1,0,0)== 255 ||
          image((unsigned int)fv->point().x(),  (unsigned int)fv->point().y()-1,0,0)== 255 ||
          image((unsigned int)fv->point().x()+1,(unsigned int)fv->point().y()-1,0,0)== 255 ||
          image((unsigned int)fv->point().x()+1,(unsigned int)fv->point().y(),0,0)  == 255) 
        {
          fv->info().set_flag(true);
        }
      }

      else if((unsigned int)fv->point().x() == 0 && (unsigned int)fv->point().y() > 0  && (unsigned int)fv->point().y() < image.dimy()) // left band
      {
        if (image((unsigned int)fv->point().x(),  (unsigned int)fv->point().y()-1,0,0)== 255 ||
          image((unsigned int)fv->point().x()+1,(unsigned int)fv->point().y()-1,0,0)== 255 ||
          image((unsigned int)fv->point().x()+1,(unsigned int)fv->point().y(),0,0)  == 255 ||
          image((unsigned int)fv->point().x()+1,(unsigned int)fv->point().y()+1,0,0)== 255 ||
          image((unsigned int)fv->point().x(),  (unsigned int)fv->point().y()+1,0,0)== 255) 
        {
          fv->info().set_flag(true);
        }
      }

      else if((unsigned int)fv->point().x() == image.dimx() && (unsigned int)fv->point().y() > 0  && (unsigned int)fv->point().y() < image.dimy()) // right band
      {
        if (image((unsigned int)fv->point().x(),  (unsigned int)fv->point().y()-1,0,0)== 255 ||
          image((unsigned int)fv->point().x()-1,(unsigned int)fv->point().y()-1,0,0)== 255 ||
          image((unsigned int)fv->point().x()-1,(unsigned int)fv->point().y(),0,0)  == 255 ||
          image((unsigned int)fv->point().x()-1,(unsigned int)fv->point().y()+1,0,0)== 255 ||
          image((unsigned int)fv->point().x(),  (unsigned int)fv->point().y()+1,0,0)== 255) 
        {
          fv->info().set_flag(true);
        }
      }

      else // middle of the image corner and bands excluded
      {
        if (image((unsigned int)fv->point().x()-1,(unsigned int)fv->point().y()-1,0,0)== 255 ||
          image((unsigned int)fv->point().x()-1,(unsigned int)fv->point().y(),0,0)  == 255 ||
          image((unsigned int)fv->point().x()-1,(unsigned int)fv->point().y()+1,0,0)== 255 ||
          image((unsigned int)fv->point().x(),  (unsigned int)fv->point().y()+1,0,0)== 255 ||
          image((unsigned int)fv->point().x(),  (unsigned int)fv->point().y()-1,0,0)== 255 ||
          image((unsigned int)fv->point().x()+1,(unsigned int)fv->point().y()-1,0,0)== 255 ||
          image((unsigned int)fv->point().x()+1,(unsigned int)fv->point().y(),0,0)  == 255 ||
          image((unsigned int)fv->point().x()+1,(unsigned int)fv->point().y()+1,0,0)== 255)
        {
          fv->info().set_flag(true);
        }
      }
    }
  }


  // this function will flag on the input image the vertices on the border 
  CImg <unsigned char> image_with_vertex_on_border(CImg <unsigned char> image) 
  {
    CImg <unsigned char> result = image;

    Finite_vertices_iterator fv = this->finite_vertices_begin();
    for(; fv != this->finite_vertices_end(); ++fv)
    {
      if(fv->info().get_flag())
      { //flag is in Alpha
        result((unsigned int)fv->point().x(),(unsigned int)fv->point().y(),0,3)= 255;
      }
    }
    return result;
  }


  // draw only points near detected borders 
  void add_constraints(CImg <unsigned char> image)
  {
    Point_2 origin_vertex;
    Point_2 end_vertex;

    Finite_vertices_iterator fv = this->finite_vertices_begin();
    for(; fv != this->finite_vertices_end(); ++fv)
    {
      if (fv->info().get_flag()) // returns if on edge or not
      {
        origin_vertex = fv->point();
        while (image((unsigned int)origin_vertex.x()-1,(unsigned int)origin_vertex.y()-1,0,0)== 255 ||
          image((unsigned int)origin_vertex.x()-1,(unsigned int)origin_vertex.y(),0,0)  == 255 ||
          image((unsigned int)origin_vertex.x()-1,(unsigned int)origin_vertex.y()+1,0,0)== 255 ||
          image((unsigned int)origin_vertex.x(),  (unsigned int)origin_vertex.y()+1,0,0)== 255 ||
          image((unsigned int)origin_vertex.x(),  (unsigned int)origin_vertex.y()-1,0,0)== 255 ||
          image((unsigned int)origin_vertex.x()+1,(unsigned int)origin_vertex.y()-1,0,0)== 255 ||
          image((unsigned int)origin_vertex.x()+1,(unsigned int)origin_vertex.y(),0,0)  == 255 ||
          image((unsigned int)origin_vertex.x()+1,(unsigned int)origin_vertex.y()+1,0,0)== 255)
        {
          if (image((unsigned int)origin_vertex.x()-1,(unsigned int)origin_vertex.y()-1,0,0)== 255)
          {//upper left
            if(image((unsigned int)origin_vertex.x()-1,(unsigned int)origin_vertex.y()-1,0,3)==255)
            { 
              end_vertex((unsigned int)origin_vertex.x()-1,(unsigned int)origin_vertex.y()-1); 
              this->insert_constraint(origin_vertex,end_vertex);
              break;
            }
            else
            {
              origin_vertex.x()--;
              origin_vertex.y()--; 
            }
          }
          else if(image((unsigned int)origin_vertex.x()-1,(unsigned int)origin_vertex.y(),0,0)  == 255)
          {//middle left
            if(image((unsigned int)origin_vertex.x()-1,(unsigned int)origin_vertex.y(),0,3)==255)
            { 
              end_vertex((unsigned int)origin_vertex.x()-1,(unsigned int)origin_vertex.y()); 
              this->insert_constraint(origin_vertex,end_vertex);
              break;
            }
            else
            {
              origin_vertex.x()--; 
            }
          }
          else if(image((unsigned int)origin_vertex.x()-1,(unsigned int)origin_vertex.y()+1,0,0)== 255)
          {//lower left
            if(image((unsigned int)origin_vertex.x()-1,(unsigned int)origin_vertex.y()+1,0,3)==255)
            { 
              end_vertex((unsigned int)origin_vertex.x()-1,(unsigned int)origin_vertex.y()+1);
              this->insert_constraint(origin_vertex,end_vertex);
              break;
            }
            else
            {
              origin_vertex.x()--;
              origin_vertex.y()++; 
            }
          }

          else if(image((unsigned int)origin_vertex.x(),(unsigned int)origin_vertex.y()+1,0,0)== 255)
          {//lower middle
            if(image((unsigned int)origin_vertex.x(),(unsigned int)origin_vertex.y()+1,0,3)==255)
            { 
              end_vertex((unsigned int)origin_vertex.x(),(unsigned int)origin_vertex.y()+1);
              this->insert_constraint(origin_vertex,end_vertex);
              break;
            }
            else
            {
              origin_vertex.y()++; 
            }
          }
          else if(image((unsigned int)origin_vertex.x(),(unsigned int)origin_vertex.y()-1,0,0)== 255)
          {//upper middle
            if(image((unsigned int)origin_vertex.x(),(unsigned int)origin_vertex.y()-1,0,3)==255)
            { 
              end_vertex((unsigned int)origin_vertex.x(),(unsigned int)origin_vertex.y()-1);
              this->insert_constraint(origin_vertex,end_vertex);
              break;
            }
            else
            {
              origin_vertex.y()--; 
            }
          }

          else if(image((unsigned int)origin_vertex.x()+1,(unsigned int)origin_vertex.y()-1,0,0)== 255)
          {//upper right
            if(image((unsigned int)origin_vertex.x()+1,(unsigned int)origin_vertex.y()-1,0,3)==255)
            {
              end_vertex((unsigned int)origin_vertex.x()+1,(unsigned int)origin_vertex.y()-1);
              this->insert_constraint(origin_vertex,end_vertex);
              break;
            }
            else
            {
              origin_vertex.x()++;
              origin_vertex.y()--; 
            }
          }

          else if(image((unsigned int)origin_vertex.x()+1,(unsigned int)origin_vertex.y(),0,0)  == 255)
          {//middle right
            if(image((unsigned int)origin_vertex.x()+1,(unsigned int)origin_vertex.y(),0,3)==255)
            { 
              end_vertex((unsigned int)origin_vertex.x()+1,(unsigned int)origin_vertex.y());
              this->insert_constraint(origin_vertex,end_vertex);
              break;
            }
            else
            {
              origin_vertex.x()++;
            }
          }

          else
          {//lower right
            if(image((unsigned int)fv->point().x()+1,(unsigned int)fv->point().y()+1,0,3)==255)
            { 
              end_vertex((unsigned int)fv->point().x()+1,(unsigned int)fv->point().y()+1);
              this->insert_constraint(origin_vertex,end_vertex);
              break;
            }
            else
            {
              origin_vertex.x()++;
              origin_vertex.y()++; 
            }
          }
        }
      }
    }
  }          


  /// Get the region of interest, ignoring the outliers.
  /// This method is used to define the OpenGL arcball sphere.
  Sphere region_of_interest() const
  {
    // A good candidate is a sphere containing the dense region of the point cloud:
    // - center point is barycenter
    // - Radius is 2 * standard deviation
    float radius = 2.f * (float)m_standard_deviation;
    return Sphere(m_barycenter, radius*radius);
  }

  /// Update region of interest.
  /// Owner is responsible to call this function after modifying the triangulation.
  void update_bounding_box()
  {
    // Update bounding box and barycenter.
    // TODO: we should use the functions in PCA component instead.
    FT xmin,xmax,ymin,ymax,zmin,zmax;
    xmin = ymin = zmin =  1e38;
    xmax = ymax = zmax = -1e38;
    Vector_3 v = CGAL::NULL_VECTOR;
    FT norm = 0;
    assert(points_begin() != points_end());
    Finite_vertices_iterator fv = this->finite_vertices_begin();
    for(; fv != this->finite_vertices_end(); ++fv)
    {
      const Point_3& p = fv->info();

      // update bbox
      xmin = (std::min)(p.x(),xmin);
      ymin = (std::min)(p.y(),ymin);
      zmin = (std::min)(p.z(),zmin);
      xmax = (std::max)(p.x(),xmax);
      ymax = (std::max)(p.y(),ymax);
      zmax = (std::max)(p.z(),zmax);

      // update barycenter
      v = v + (p - CGAL::ORIGIN);
      norm += 1;
    }
    //
    Point_3 p(xmin,ymin,zmin);
    Point_3 q(xmax,ymax,zmax);
    m_bounding_box = Iso_cuboid_3(p,q);
    //
    m_barycenter = CGAL::ORIGIN + v / norm;

    /// Compute standard deviation
    Geom_traits::Compute_squared_distance_3 sqd;
    FT sq_radius = 0;
    /*Finite_vertices_iterator*/ fv = this->finite_vertices_begin();
    for(; fv != this->finite_vertices_end(); ++fv)
    {
      const Point_3& p = fv->info();
      sq_radius += sqd(p, m_barycenter);
    }
    sq_radius /= number_of_vertices();
    m_standard_deviation = CGAL::sqrt(sq_radius);
  }



  // draw 2D cdt vertices
  void gl_draw_2D_vertices(const unsigned char r, const unsigned char g,
    const unsigned char b, float size)
  {
    ::glPointSize(size);
    ::glColor3ub(r,g,b);
    ::glBegin(GL_POINTS);

    Finite_vertices_iterator fv = this->finite_vertices_begin();
    for(; fv != this->finite_vertices_end(); ++fv)
    {
      ::glVertex2d(fv->point().x(),fv->point().y());
    }
    ::glEnd();
  }


  // draw 2D only points near detected borders 
  void gl_draw_on_border_2D_vertices(const unsigned char r, const unsigned char g,
    const unsigned char b, float size, CImg <unsigned char> image)
  {
    ::glPointSize(size);
    ::glColor3ub(r,g,b);
    ::glBegin(GL_POINTS);

    Finite_vertices_iterator fv = this->finite_vertices_begin();
    for(; fv != this->finite_vertices_end(); ++fv)
    {
      // if pixel any of 9x9 surrounding pixels is on border keep vertex(white color is used in frei-chen gradient operator)
      if (fv->info().get_flag())
      {
        ::glVertex2d(fv->point().x(),fv->point().y());
      }
    }
    ::glEnd();
  }
    

  // draw 2D cdt constrained edges
  void gl_draw_2D_constrained_edges(unsigned char r,unsigned char g,
    unsigned char b,float line_width)
  {
    ::glColor3ub(r,g,b);
    ::glLineWidth(line_width);
    ::glBegin(GL_LINES);
    Finite_edges_iterator fe = this->finite_edges_begin();
    for(; fe != this->finite_edges_end(); ++fe)
    {
      if(fe->first->is_constrained(fe->second))
      {
        Point p1 = fe->first->vertex(ccw(fe->second))->point();
        Point p2 = fe->first->vertex(cw(fe->second))->point();
        ::glVertex2d(p1.x(),p1.y());
        ::glVertex2d(p2.x(),p2.y());
      }
    }
    ::glEnd();
  }


  // draw 2D cdt constrained delaunay triangles
  void gl_draw_2D_constrained_delaunay_triangles(const unsigned char r, const unsigned char g,
    const unsigned char b)
  {

      ::glColor3ub(r,g,b);
      ::glBegin(GL_TRIANGLES);

      Finite_faces_iterator ff = this->finite_faces_begin();
      for(; ff != this->finite_faces_end(); ++ff)
      {
        Vertex_handle v1 = ff->vertex(0);
        Vertex_handle v2 = ff->vertex(1);
        Vertex_handle v3 = ff->vertex(2);
        ::glVertex2d(v1->point().x(), v1->point().y());
        ::glVertex2d(v2->point().x(), v2->point().y());
        ::glVertex2d(v3->point().x(), v3->point().y());
      } 
      ::glEnd();
  } 


  // 3D projection of the tracked 2D points
  void gl_draw_soup_vertices(const unsigned char r, const unsigned char g,
    const unsigned char b, float size)
  {
    ::glPointSize(size);
    ::glColor3ub(r,g,b);
    ::glBegin(GL_POINTS);

    Finite_vertices_iterator fv = this->finite_vertices_begin();
    for(; fv != this->finite_vertices_end(); ++fv)
    {
      const Point_3& p = fv->info().get_point3();
      ::glVertex3d(p.x(),p.y(),p.z());
    }

    ::glEnd();
  }

  // 3D projection of the tracked 2D constrained edges
  void gl_draw_soup_constrained_edges(const unsigned char r, const unsigned char g,
    const unsigned char b, const float width)
  {

    ::glLineWidth(width);
    ::glColor3ub(r,g,b);
    ::glBegin(GL_LINES);

    Finite_edges_iterator fe = this->finite_edges_begin();
    for(; fe != this->finite_edges_end(); ++fe)
    {
      if(fe->first->is_constrained(fe->second))
      {
        Point_3 p1 = fe->first->vertex(ccw(fe->second))->info().get_point3();
        Point_3 p2 = fe->first->vertex(cw(fe->second))->info().get_point3();
        ::glVertex3d(p1.x(), p1.y(), p1.z());
        ::glVertex3d(p2.x(), p2.y(), p2.z());
      }

    }

    ::glEnd();
  }

  // 3D projection of the tracked 2D constrained triangulation
  void gl_draw_soup_constrained_triangles(const unsigned char r, const unsigned char g,
    const unsigned char b){

      ::glColor3ub(r,g,b);
      ::glBegin(GL_TRIANGLES);

      Finite_faces_iterator ff = this->finite_faces_begin();
      for(; ff != this->finite_faces_end(); ++ff)
      {
        Vertex_handle v1 = ff->vertex(0);
        Vertex_handle v2 = ff->vertex(1);
        Vertex_handle v3 = ff->vertex(2);
        Point_3 p1 = v1->info().get_point3();
        Point_3 p2 = v2->info().get_point3();
        Point_3 p3 = v3->info().get_point3();
        ::glVertex3d(p1.x(), p1.y(), p1.z());
        ::glVertex3d(p2.x(), p2.y(), p2.z());
        ::glVertex3d(p3.x(), p3.y(), p3.z());
      } 
      ::glEnd();
  } 

};

#endif // _Gyroviz_dt2_
