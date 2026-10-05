#include "modeling/Boolean.h"
#include "core/geometry/GeometricTolerance.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>
#include <vector>
namespace microsw::modeling { namespace {
using Key=std::array<std::size_t,3>;
struct Box{std::array<double,3> lo{},hi{};}; struct Cell{Key index{};bool occupied{};}; struct Rectangle{int axis{},sign{};Key lo{},hi{};};
std::optional<Box> recognizeBox(const topology::Solid&s){
 if(!s.isValid()||s.shells().size()!=1||s.vertices().size()!=8||s.edges().size()!=12||s.faces().size()!=6)return std::nullopt;
 Box b;b.lo.fill(std::numeric_limits<double>::infinity());b.hi.fill(-std::numeric_limits<double>::infinity());
 for(const auto&v:s.vertices()){const auto&p=v.point();std::array<double,3>a{p.x(),p.y(),p.z()};for(int i=0;i<3;++i){if(!std::isfinite(a[i]))return std::nullopt;b.lo[i]=std::min(b.lo[i],a[i]);b.hi[i]=std::max(b.hi[i],a[i]);}}
 for(int i=0;i<3;++i)if(b.hi[i]-b.lo[i]<=geometry::defaultGeometricTolerance)return std::nullopt;
 std::set<Key> corners;for(const auto&v:s.vertices()){const auto&p=v.point();std::array<double,3>a{p.x(),p.y(),p.z()};Key k{};for(int i=0;i<3;++i){if(std::abs(a[i]-b.lo[i])<=geometry::defaultGeometricTolerance)k[i]=0;else if(std::abs(a[i]-b.hi[i])<=geometry::defaultGeometricTolerance)k[i]=1;else return std::nullopt;}corners.insert(k);}if(corners.size()!=8)return std::nullopt;
 for(const auto&f:s.faces()){const auto&n=f.supportPlane().normal();if((std::abs(n.x())>.5)+(std::abs(n.y())>.5)+(std::abs(n.z())>.5)!=1)return std::nullopt;}return b;}
bool ambiguous(const Box&a,const Box&b){for(int i=0;i<3;++i)for(double x:{a.lo[i],a.hi[i]})for(double y:{b.lo[i],b.hi[i]}){double d=std::abs(x-y);if(d>0&&d<=geometry::defaultGeometricTolerance)return true;}return false;}
std::vector<double> coords(double a,double A,double b,double B){std::vector<double>v{a,A,b,B};std::sort(v.begin(),v.end());v.erase(std::unique(v.begin(),v.end(),[](double x,double y){return std::abs(x-y)<=geometry::defaultGeometricTolerance;}),v.end());return v;}
size_t flat(const Key&k,const std::array<size_t,3>&n){return(k[0]*n[1]+k[1])*n[2]+k[2];}
topology::Solid reconstruct(const std::array<std::vector<double>,3>&c,const std::vector<Cell>&cells,const std::array<size_t,3>&n){
 std::vector<Rectangle>rs;for(const auto&cell:cells)if(cell.occupied)for(int axis=0;axis<3;++axis)for(int sign:{-1,1}){int q=static_cast<int>(cell.index[axis])+sign;bool other=false;if(q>=0&&q<static_cast<int>(n[axis])){auto k=cell.index;k[axis]=static_cast<size_t>(q);other=cells[flat(k,n)].occupied;}if(!other){Rectangle r{axis,sign,cell.index,{cell.index[0]+1,cell.index[1]+1,cell.index[2]+1}};auto plane=cell.index[axis]+(sign>0?1:0);r.lo[axis]=r.hi[axis]=plane;rs.push_back(r);}}
 topology::Solid s;std::map<Key,topology::VertexId>vs;std::map<std::pair<Key,Key>,topology::EdgeId>es;
 auto vertex=[&](Key k){auto[it,newv]=vs.try_emplace(k);if(newv)it->second=s.addVertex({c[0][k[0]],c[1][k[1]],c[2][k[2]]});return it->second;};
 auto edge=[&](Key a,Key b){auto mm=std::minmax(a,b);auto key=std::pair{mm.first,mm.second};auto[it,newe]=es.try_emplace(key);if(newe)it->second=s.addEdge(vertex(key.first),vertex(key.second));return topology::OrientedEdgeUse{it->second,a==key.first?topology::EdgeOrientation::Forward:topology::EdgeOrientation::Reversed};};
 std::vector<topology::OrientedFaceUse>fs;for(const auto&r:rs){int u=(r.axis+1)%3,v=(r.axis+2)%3;Key p0=r.lo,p1=r.lo,p2=r.lo,p3=r.lo;p1[u]=r.hi[u];p2[u]=r.hi[u];p2[v]=r.hi[v];p3[v]=r.hi[v];std::array<Key,4>p{p0,p1,p2,p3};if(r.sign<0)std::reverse(p.begin(),p.end());auto w=s.addWire({edge(p[0],p[1]),edge(p[1],p[2]),edge(p[2],p[3]),edge(p[3],p[0])});std::array<double,3>o{c[0][p0[0]],c[1][p0[1]],c[2][p0[2]]},normal{};normal[r.axis]=r.sign;auto f=s.addFace(w,{{o[0],o[1],o[2]},{normal[0],normal[1],normal[2]}});fs.push_back({f,topology::FaceOrientation::Forward});}auto shell=s.addShell(std::move(fs));s.setCellDerivedRootShell(shell);return s;}
}
BooleanResult BooleanResult::success(topology::Solid s){return{BooleanStatus::SUCCESS,std::move(s)};}BooleanResult BooleanResult::failure(BooleanStatus s){if(s==BooleanStatus::SUCCESS)throw std::invalid_argument{"SUCCESS requires Solid"};return{s,std::nullopt};}
BooleanResult booleanOperation(const topology::Solid&a,const topology::Solid&b,BooleanOperation op)noexcept{try{
 auto A=recognizeBox(a),B=recognizeBox(b);if(!A||!B)return BooleanResult::failure(BooleanStatus::INVALID_INPUT);if(ambiguous(*A,*B))return BooleanResult::failure(BooleanStatus::UNSUPPORTED_CASE);
 std::array<double,3>ov{};for(int i=0;i<3;++i)ov[i]=std::min(A->hi[i],B->hi[i])-std::max(A->lo[i],B->lo[i]);bool sep=std::any_of(ov.begin(),ov.end(),[](double x){return x< -geometry::defaultGeometricTolerance;});bool touch=!sep&&std::any_of(ov.begin(),ov.end(),[](double x){return x<=geometry::defaultGeometricTolerance;});if(touch)return BooleanResult::failure(BooleanStatus::UNSUPPORTED_CASE);if(sep&&op==BooleanOperation::Intersection)return BooleanResult::failure(BooleanStatus::EMPTY);if(sep&&op==BooleanOperation::Union)return BooleanResult::failure(BooleanStatus::UNSUPPORTED_CASE);
 std::array<std::vector<double>,3>c;std::array<size_t,3>n{};for(int i=0;i<3;++i){c[i]=coords(A->lo[i],A->hi[i],B->lo[i],B->hi[i]);n[i]=c[i].size()-1;}std::vector<Cell>cells;
 for(size_t x=0;x<n[0];++x)for(size_t y=0;y<n[1];++y)for(size_t z=0;z<n[2];++z){Key k{x,y,z};std::array<double,3>m{(c[0][x]+c[0][x+1])/2,(c[1][y]+c[1][y+1])/2,(c[2][z]+c[2][z+1])/2};bool ia=true,ib=true;for(int q=0;q<3;++q){ia&=m[q]>A->lo[q]&&m[q]<A->hi[q];ib&=m[q]>B->lo[q]&&m[q]<B->hi[q];}cells.push_back({k,op==BooleanOperation::Union?(ia||ib):op==BooleanOperation::Intersection?(ia&&ib):(ia&&!ib)});}
 size_t count=std::count_if(cells.begin(),cells.end(),[](const auto&x){return x.occupied;});if(!count)return BooleanResult::failure(BooleanStatus::EMPTY);std::vector<bool>seen(cells.size());std::queue<Key>q;auto first=std::find_if(cells.begin(),cells.end(),[](const auto&x){return x.occupied;});q.push(first->index);seen[flat(first->index,n)]=true;size_t reached=0;while(!q.empty()){auto k=q.front();q.pop();++reached;for(int axis=0;axis<3;++axis)for(int d:{-1,1}){int p=static_cast<int>(k[axis])+d;if(p<0||p>=static_cast<int>(n[axis]))continue;auto next=k;next[axis]=static_cast<size_t>(p);auto f=flat(next,n);if(cells[f].occupied&&!seen[f]){seen[f]=true;q.push(next);}}}if(reached!=count)return BooleanResult::failure(BooleanStatus::UNSUPPORTED_CASE);
 std::vector<bool> exterior(cells.size());std::queue<Key> emptyQueue;
 for(const auto& cell:cells)if(!cell.occupied&&(cell.index[0]==0||cell.index[1]==0||cell.index[2]==0||cell.index[0]+1==n[0]||cell.index[1]+1==n[1]||cell.index[2]+1==n[2])){auto f=flat(cell.index,n);if(!exterior[f]){exterior[f]=true;emptyQueue.push(cell.index);}}
 while(!emptyQueue.empty()){auto k=emptyQueue.front();emptyQueue.pop();for(int axis=0;axis<3;++axis)for(int d:{-1,1}){int p=static_cast<int>(k[axis])+d;if(p<0||p>=static_cast<int>(n[axis]))continue;auto next=k;next[axis]=static_cast<size_t>(p);auto f=flat(next,n);if(!cells[f].occupied&&!exterior[f]){exterior[f]=true;emptyQueue.push(next);}}}
 for(size_t i=0;i<cells.size();++i)if(!cells[i].occupied&&!exterior[i])return BooleanResult::failure(BooleanStatus::UNSUPPORTED_CASE);
 auto result=reconstruct(c,cells,n);long long chi=static_cast<long long>(result.vertices().size())-static_cast<long long>(result.edges().size())+static_cast<long long>(result.faces().size());if(chi!=2)return BooleanResult::failure(BooleanStatus::UNSUPPORTED_CASE);return BooleanResult::success(std::move(result));
 }catch(...){return BooleanResult::failure(BooleanStatus::NUMERICAL_FAILURE);}}
}
