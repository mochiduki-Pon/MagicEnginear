#include	"wall.h"	
#include	"../system/PlaneDrawer.h"

PLANEINFO wall::getplaneinfo() {
	return m_plane.GetPlaneInfo();
}

float wall::getwidth() const {
	return m_width;
}

float wall::getheight() const {
	return m_height;
}

void wall::makeplanedata() 
{
	Matrix4x4 wallr = Matrix4x4::CreateRotationY(m_srt.rot.y);
	Matrix4x4 wallt = Matrix4x4::CreateTranslation(m_srt.pos);
	Matrix4x4 mtx = wallr * wallt;

	// ‚S’¸“_‚ðŒvŽZ
	for (unsigned int y = 0; y < 2; y++) {
		Vector3	vtx{};
		for (unsigned int x = 0; x < 2; x++) {

			// ’¸“_À•WƒZƒbƒg
			vtx.x = -m_width / 2.0f + x * m_width;
			vtx.y = -m_height / 2.0f + y * m_height;
			vtx.z = 0.0f;

			// •Ç‚Ì‰ñ“]î•ñ‚ð”½‰f‚³‚¹‚é
			vtx = Vector3::Transform(vtx, mtx);

			m_vertices.push_back(vtx);
		}
	}

	// •½–Ê‚Ì•û’öŽ®‚ðì‚é
	m_plane.MakeEquatation(
		m_vertices[0],
		m_vertices[3],
		m_vertices[1]);

}

void wall::init() 
{
	// •½–Ê‚Ì•û’öŽ®ƒf[ƒ^‚ðì¬
	makeplanedata();

	// •½–Ê•`‰æ‰Šú‰»
	PlaneDrawerInit();

}

void wall::update(uint64_t dt) {


}

void wall::draw(uint64_t dt) 
{
	PlaneDrawerDraw(
		m_srt.rot, 
		m_width, 
		m_height, 
		Color(1, 0, 0, 0.4f), 
		m_srt.pos.x, 
		m_srt.pos.y,
		m_srt.pos.z);
}

void wall::dispose() {

	// •½–Ê•`‰æ‰Šú‰»
	

}
