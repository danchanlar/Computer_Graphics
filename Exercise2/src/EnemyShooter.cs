using System.Collections;
using UnityEngine;

public class EnemyShooter : MonoBehaviour
{
    public GameObject bulletPrefab;
    public float minShootDelay = 1.0f;
    public float maxShootDelay = 5.0f;

    private void Start()
    {
        StartCoroutine(ShootRoutine());
    }

    private IEnumerator ShootRoutine()
    {
        while (true)
        {
            if (GameManager.Instance != null && GameManager.Instance.IsGameOver())
                yield break;

            float waitTime = Random.Range(minShootDelay, maxShootDelay);
            yield return new WaitForSeconds(waitTime);

            Shoot();
        }
    }

    private void Shoot()
    {
        if (bulletPrefab == null) return;

        Vector3 spawnPos = transform.position;
        Quaternion rot = Quaternion.identity;

        Instantiate(bulletPrefab, spawnPos, rot);
    }
}
