using UnityEngine;

public class ShooterBullet : MonoBehaviour
{
    public float lifeTime = 5f;

    [Header("Effects")]
    public AudioClip hitEnemyClip;
    public GameObject hitEnemyVFX;

    private void Start()
    {
        Destroy(gameObject, lifeTime);
    }

    private void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("Enemy"))
        {
            // Βρες πόσους πόντους αξίζει αυτός ο εχθρός
            Score Score = other.GetComponent<Score>();
            if (Score != null && GameManager.Instance != null)
            {
                GameManager.Instance.AddScore(Score.scoreValue);
            }

            // Ήχος
            if (hitEnemyClip != null)
            {
                AudioSource.PlayClipAtPoint(hitEnemyClip, transform.position);
            }

            // VFX
            if (hitEnemyVFX != null)
            {
                Instantiate(hitEnemyVFX, other.transform.position, Quaternion.identity);
            }

            // Καταστροφή εχθρού & σφαίρας
            Destroy(other.gameObject);
            Destroy(gameObject);
        }
        else if (other.CompareTag("Ground"))
        {
            Destroy(gameObject);
        }
    }
}
