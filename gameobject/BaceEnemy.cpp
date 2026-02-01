//#include "BaceEnemy.h"
//#include "../scene/TutorialScene.h"
//
//// 壁とのヒットチェック
//bool BaceEnemy::wallshitcheck(std::vector<wall::WallCollision>& hitwalls)
//{
//    bool hitflag = false;
//
//    const Vector3 nextpos = m_srt.pos + m_move;
//    const float radius = 10.0f; // プレイヤ半径
//
//    // 壁を取得
//    std::vector<wall*> walls{};
//    walls = ((TutorialScene*)m_ownerscene)->getwalls(); // 使うScene
//
//    for (auto* w : walls)
//    {
//        if (!w) continue;
//
//        w->clearhitflag();
//
//        // 壁OBB（厚みを持たせる）
//        const SRT wsrt = w->getSRT();
//        const float ww = w->getwidth();
//        const float wh = w->getheight();
//        constexpr float WALL_THICK = 2.0f;
//
//        Call::Collision::BoundingBoxOBB wallObb =
//            Call::Collision::SetOBB(wsrt.rot, wsrt.pos, ww, wh, WALL_THICK);
//
//        // プレイヤ球（移動先）
//        Call::Collision::BoundingSphere sphere(nextpos, radius);
//
//        // 球 vs OBB
//        if (!Call::Collision::CollisionSphereOBB(sphere, wallObb))
//            continue;
//
//        // 最近接点
//        wall::WallCollision wc{};
//        wc.wall = w;
//        wc.Penetration = Vector3(0, 0, 0);
//        wc.Sliding = Vector3(0, 0, 0);
//        Call::Collision::ClosestPtPointOBB(sphere.center, wallObb, wc.IntersectionPoint);
//
//        // 押し戻しベクトル（最小侵入解消）
//        Vector3 v = sphere.center - wc.IntersectionPoint;
//        const float d2 = v.LengthSquared();
//
//        if (d2 < 1e-6f)
//        {
//            const PLANEINFO pi = w->getplaneinfo();
//            Vector3 n(pi.plane.a, pi.plane.b, pi.plane.c);
//            if (n.LengthSquared() < 1e-6f) n = Vector3(0, 0, 1);
//            n.Normalize();
//            wc.Penetration = n * radius;
//        }
//        else
//        {
//            const float d = std::sqrt(d2);
//            v *= (1.0f / d);                 // n
//            const float pen = (radius - d);  // 侵入量
//            wc.Penetration = v * pen;
//        }
//
//        // スライド量
//        Vector3 n = wc.Penetration;
//        if (n.LengthSquared() > 1e-6f) n.Normalize();
//
//        // move の法線方向成分を取り除く = 壁面に沿う
//        wc.Sliding = m_move - n * m_move.Dot(n);
//
//        w->sethitflag();
//        hitwalls.push_back(wc);
//        hitflag = true;
//    }
//
//    return hitflag;
//}
//
//void BaceEnemy::SeparateFromOthers()
//{
//    auto* scn = static_cast<TutorialScene*>(m_ownerscene);
//    if (!scn) return;
//
//    const auto& enemies = scn->GetEnemies();
//
//    constexpr float SEP_R = 40.0f;  // 分離半径（この距離未満なら押し返す）
//    constexpr float MAX_PUSH = 1.5f;   // 1フレームで動ける最大押し量
//    constexpr float K = 0.15f;  // 押しの強さ（重なり量に掛ける係数）
//
//    Vector3 push(0, 0, 0);
//
//    for (const auto& e : enemies)
//    {
//        if (!e) continue;
//        //if (e.get() == this) continue;
//        if (!e->IsAlive()) continue;
//
//        Vector3 d = m_srt.pos - e->getSRT().pos;
//        d.y = 0.0f; // 水平だけで押す
//
//        const float d2 = d.LengthSquared();
//        if (d2 < 1e-6f) continue; // ほぼ同一点は方向が取れないのでスキップ
//
//        const float dist = std::sqrt(d2);
//        if (dist >= SEP_R) continue; // 遠いなら関係なし
//
//        // 押し方向（相手→自分）
//        const Vector3 n = d * (1.0f / dist);
//
//        // 重なり量（近いほど大きい）
//        const float overlap = (SEP_R - dist);
//
//        // 重なりに比例して押す（ガクッとしにくい）
//        push += n * (overlap * K);
//    }
//
//    // push の長さを MAX_PUSH にクランプ（向きは維持）
//    const float len2 = push.LengthSquared();
//    const float max2 = MAX_PUSH * MAX_PUSH;
//    if (len2 > max2)
//    {
//        push *= (MAX_PUSH / std::sqrt(len2));
//    }
//
//    m_srt.pos += push;
//}
//
//void BaceEnemy::update(uint64_t delta)
//{
//}
//void BaceEnemy::draw(uint64_t delta)
//{
//}
//void BaceEnemy::init() {}
//void BaceEnemy::dispose() {}