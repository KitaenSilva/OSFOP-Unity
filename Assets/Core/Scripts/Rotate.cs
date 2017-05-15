using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Rotate : MonoBehaviour {
	[SerializeField]
	public float speed = 1.0f;
	[SerializeField]
	public Vector3 angle = new Vector3 (1.0f, 0.0f, 0.0f);

	
	// Update is called once per frame
	void Update () {
		gameObject.transform.Rotate (angle, Time.deltaTime * speed);
	}
}
