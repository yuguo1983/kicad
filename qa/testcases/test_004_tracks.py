import unittest
import pcbnew

class TestTracks(unittest.TestCase):

    def setUp(self):
        self.pcb = pcbnew.LoadBoard("data/pcbnew/tracks_arcs_vias.kicad_pcb")

    def test_tracks(self):
        tracks = [t for t in self.pcb.Tracks() if t.GetClass() == 'PCB_TRACK']
        self.assertEqual(16, len(tracks))
        track = sorted(tracks, key=lambda t: [t.GetStart()[0], t.GetStart()[1]])[0]
        self.assertEqual([27000000, 27585787], [track.GetStart()[0], track.GetStart()[1]])
        self.assertEqual([27000000, 26500000], [track.GetEnd()[0], track.GetEnd()[1]])
        self.assertEqual(250000, track.GetWidth())
        self.assertEqual('McNetty', track.GetNetname())

        dup_track = track.Duplicate()
        self.assertTrue( dup_track.m_Uuid != track.m_Uuid )


    def test_arcs(self):
        arcs = [t.Cast() for t in self.pcb.Tracks() if t.GetClass() == 'PCB_ARC']
        self.assertEqual(13, len(arcs))
        arc = sorted(arcs, key=lambda t: [t.GetStart()[0], t.GetStart()[1]])[0]
        self.assertEqual([29414200, 26500000], [arc.GetCenter()[0], arc.GetCenter()[1]])
        self.assertEqual([1800, 2250], [round(arc.GetArcAngleStart().AsTenthsOfADegree()),
                                        round(arc.GetArcAngleEnd().AsTenthsOfADegree())])
        self.assertEqual(2414200, round(arc.GetRadius()))

        dup_arc = arc.Duplicate()
        self.assertTrue( dup_arc.m_Uuid != arc.m_Uuid )

    def test_vias(self):
        vias = [t.Cast() for t in self.pcb.Tracks() if t.GetClass() == 'PCB_VIA']
        self.assertEqual(2, len(vias))
        via = sorted(vias, key=lambda t: [t.GetStart()[0], t.GetStart()[1]])[0]
        self.assertEqual([29000000, 41000000], [via.GetStart()[0], via.GetStart()[1]])
        self.assertEqual(400000, via.GetDrillValue())

        dup_via = via.Duplicate()
        self.assertTrue( dup_via.m_Uuid != via.m_Uuid )