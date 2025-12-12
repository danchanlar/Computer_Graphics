using UnityEngine;

public class EnemyBullet : MonoBehaviour
{
    public float speed = 20f;
    public float lifeTime = 5f;

    [Header("Effects")]
    public AudioClip hitShooterClip;
    public GameObject hitShooterVFX;   

    private void Start()
    {
        Destroy(gameObject, lifeTime);
    }

    private void Update()
    {
        // Πέφτει προς τα κάτω
        transform.Translate(Vector3.down * speed * Time.deltaTime, Space.World);
    }

    private void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("Ground"))
        {
            Destroy(gameObject);
        }
        else if (other.CompareTag("Shooter"))
        {
            // Μικρό impact VFX πάνω στον Shooter
            if (hitShooterVFX != null)
            {
                Instantiate(hitShooterVFX, other.transform.position, Quaternion.identity);
            }

            // Ήχος χτυπήματος
            if (hitShooterClip != null)
            {
                AudioSource.PlayClipAtPoint(hitShooterClip, other.transform.position);
            }

            if (GameManager.Instance != null)
            {
                GameManager.Instance.PlayerDied();
            }

            Destroy(gameObject);
        }
    }
}
