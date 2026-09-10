import unittest
import json
import urllib.request
import urllib.error
import threading
import time

from app import app, ensure_data_files

class TestEcoPowerApplication(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        """Start Flask test server in background thread."""
        ensure_data_files()
        cls.port = 5008
        cls.server_thread = threading.Thread(
            target=lambda: app.run(host='127.0.0.1', port=cls.port, debug=False, use_reloader=False)
        )
        cls.server_thread.daemon = True
        cls.server_thread.start()
        time.sleep(1.2)  # Allow server time to bind
        cls.base_url = f"http://127.0.0.1:{cls.port}"

    def test_01_web_routes_render(self):
        """Test GET /, GET /about, and GET /hardware return HTTP 200 OK with HTML content."""
        routes = ['/', '/dashboard', '/about', '/hardware']
        for route in routes:
            res = urllib.request.urlopen(f"{self.base_url}{route}")
            self.assertEqual(res.status, 200, f"Route {route} failed")
            html = res.read().decode('utf-8')
            self.assertIn("EcoPower", html, f"Route {route} missing title")

    def test_02_api_status(self):
        """Test GET /api/status endpoint."""
        res = urllib.request.urlopen(f"{self.base_url}/api/status")
        self.assertEqual(res.status, 200)
        data = json.loads(res.read().decode('utf-8'))
        self.assertTrue(data['success'])
        self.assertIn('system_status', data)
        self.assertIn('connection_mode', data)
        self.assertIn('active_loads_count', data)

    def test_03_api_energy_get(self):
        """Test GET /api/energy endpoint."""
        res = urllib.request.urlopen(f"{self.base_url}/api/energy")
        self.assertEqual(res.status, 200)
        data = json.loads(res.read().decode('utf-8'))
        self.assertTrue(data['success'])
        self.assertIn('voltage', data)
        self.assertIn('renewable_utilization', data)

    def test_04_api_battery(self):
        """Test GET /api/battery endpoint."""
        res = urllib.request.urlopen(f"{self.base_url}/api/battery")
        self.assertEqual(res.status, 200)
        data = json.loads(res.read().decode('utf-8'))
        self.assertTrue(data['success'])
        self.assertIn('battery_pct', data)
        self.assertIn('state', data)

    def test_05_api_loads_get(self):
        """Test GET /api/loads endpoint."""
        res = urllib.request.urlopen(f"{self.base_url}/api/loads")
        self.assertEqual(res.status, 200)
        data = json.loads(res.read().decode('utf-8'))
        self.assertTrue(data['success'])
        self.assertIsInstance(data['loads'], list)
        self.assertEqual(len(data['loads']), 3)

    def test_06_post_load_control(self):
        """Test POST /api/load-control to toggle load state."""
        payload = json.dumps({
            "load_id": "load_high",
            "action": "toggle"
        }).encode('utf-8')

        req = urllib.request.Request(
            f"{self.base_url}/api/load-control",
            data=payload,
            headers={"Content-Type": "application/json"}
        )
        res = urllib.request.urlopen(req)
        self.assertEqual(res.status, 200)
        result = json.loads(res.read().decode('utf-8'))
        self.assertTrue(result['success'])
        self.assertEqual(result['load']['id'], 'load_high')

    def test_07_post_simulation_scenarios(self):
        """Test POST /api/simulation with preset hackathon scenarios."""
        scenarios = ['high_solar', 'low_solar', 'low_battery', 'normal']
        for s in scenarios:
            payload = json.dumps({"scenario": s}).encode('utf-8')
            req = urllib.request.Request(
                f"{self.base_url}/api/simulation",
                data=payload,
                headers={"Content-Type": "application/json"}
            )
            res = urllib.request.urlopen(req)
            self.assertEqual(res.status, 200)
            data = json.loads(res.read().decode('utf-8'))
            self.assertTrue(data['success'])
            self.assertEqual(data['scenario'], s)

    def test_08_energy_calculations_and_load_shedding(self):
        """Test telemetry ingestion, P=V*I formula, utilization %, and priority load shedding."""
        # Test low solar + low battery -> should shed medium and low priority loads
        payload = json.dumps({
            "voltage": 6.0,
            "current": 0.5,
            "battery_pct": 18.0,
            "energy_consumed": 10.0,
            "source": "unit_test"
        }).encode('utf-8')

        req = urllib.request.Request(
            f"{self.base_url}/api/energy",
            data=payload,
            headers={"Content-Type": "application/json"}
        )
        res = urllib.request.urlopen(req)
        self.assertEqual(res.status, 200)
        result = json.loads(res.read().decode('utf-8'))

        self.assertTrue(result['success'])
        data = result['data']
        # Power = 6.0 * 0.5 = 3.0 W
        self.assertEqual(data['power'], 3.0)
        # Check load state: High Priority ON, Med/Low OFF
        loads = {l['id']: l['status'] for l in result['loads']}
        self.assertEqual(loads['load_high'], 'ON')
        self.assertEqual(loads['load_med'], 'OFF')
        self.assertEqual(loads['load_low'], 'OFF')

    def test_09_invalid_input_handling(self):
        """Test invalid inputs return HTTP 400 Bad Request."""
        bad_payload = json.dumps({
            "voltage": -10.0,  # Negative voltage invalid
            "battery_pct": 120.0  # >100 invalid
        }).encode('utf-8')

        req = urllib.request.Request(
            f"{self.base_url}/api/energy",
            data=bad_payload,
            headers={"Content-Type": "application/json"}
        )
        with self.assertRaises(urllib.error.HTTPError) as cm:
            urllib.request.urlopen(req)

        self.assertEqual(cm.exception.code, 400)

if __name__ == '__main__':
    unittest.main()
