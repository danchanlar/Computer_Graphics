using UnityEngine;
using UnityEngine.UI;
public class GameManager : MonoBehaviour
{
    public static GameManager Instance;

    [Header("References")]
    public GameObject shooter;

    [Header("Game End Sounds")]
    public AudioClip winClip;
    public AudioClip loseClip;

    [Header("Game End VFX")]
    public GameObject winVFX;   // εφέ όταν κερδίζει ο Shooter
    public GameObject loseVFX;  // εφέ όταν χάνει ο Shooter

    [Header("Score")]
    public int score = 0;
    public Text scoreText;   // προαιρετικά, αν θες να το βλέπεις στην οθόνη


    private bool isGameOver = false;
    private bool isGameWon = false;

    private void Awake()
    {
        if (Instance == null)
            Instance = this;
        else
        {
            Destroy(gameObject);
            return;
        }
    }
    public void AddScore(int amount)
    {
        if (isGameOver || isGameWon) return;

        score += amount;
        Debug.Log("Score: " + score);

        if (scoreText != null)
        {
            scoreText.text = "Score: " + score;
        }
    }

    private void Update()
    {
        if (isGameOver || isGameWon)
            return;

        // Αν δεν υπάρχουν εχθροί → ΝΙΚΗ
        GameObject[] enemies = GameObject.FindGameObjectsWithTag("Enemy");
        if (enemies.Length == 0)
        {
            GameWin();
        }
    }

    public void PlayerDied()
    {
        if (isGameOver || isGameWon) return;

        isGameOver = true;

        // ➤ Εφέ Ήττας
        if (loseVFX != null && shooter != null)
        {
            Instantiate(loseVFX, shooter.transform.position, Quaternion.identity);
        }

        // ➤ Ήχος Ήττας
        if (loseClip != null && Camera.main != null)
        {
            AudioSource.PlayClipAtPoint(loseClip, Camera.main.transform.position);
        }

        // ➤ Καταστροφή Shooter
        if (shooter != null)
            Destroy(shooter);

        Debug.Log("❌ GAME OVER — Έχασες!");
    }

    private void GameWin()
    {
        if (isGameOver || isGameWon) return;

        isGameWon = true;

        // ➤ Εφέ Νίκης
        Vector3 pos = shooter != null ? shooter.transform.position : Vector3.zero;
        if (winVFX != null)
        {
            Instantiate(winVFX, pos, Quaternion.identity);
        }

        // ➤ Ήχος Νίκης
        if (winClip != null && Camera.main != null)
        {
            AudioSource.PlayClipAtPoint(winClip, Camera.main.transform.position);
        }

        Debug.Log("🏆 YOU WIN — Νίκησες όλους τους εχθρούς!");
    }

    public bool IsGameOver() => isGameOver;
    public bool IsGameWon() => isGameWon;
}
