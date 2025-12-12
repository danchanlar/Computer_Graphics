using System.Collections;
using UnityEngine;

public class EnemyGroupController : MonoBehaviour
{
    [Header("Movement Pattern")]
    public float stepX = 2f;        // μέγεθος βήματος στον άξονα X
    public int stepsPerSide = 5;    // πόσα βήματα θα πάει δεξιά/αριστερά
    public float stepDownY = 2f;    // πόσο κατεβαίνει κάθε κύκλο
    public float stepDuration = 0.2f; // χρόνος για κάθε βήμα

    private void Start()
    {
        StartCoroutine(MovePattern());
    }

    private IEnumerator MovePattern()
    {
        while (true)
        {
            if (GameManager.Instance != null && GameManager.Instance.IsGameOver())
                yield break;

            // 1. από κέντρο προς τα δεξιά
            yield return StartCoroutine(MoveHorizontal(stepX, stepsPerSide));
            // 2. πίσω στο κέντρο από δεξιά
            yield return StartCoroutine(MoveHorizontal(-stepX, stepsPerSide));

            // 3. από κέντρο προς τα αριστερά
            yield return StartCoroutine(MoveHorizontal(-stepX, stepsPerSide));
            // 4. πίσω στο κέντρο από αριστερά
            yield return StartCoroutine(MoveHorizontal(stepX, stepsPerSide));

            // 5. κατεβαίνουν στον y άξονα κατά 2
            yield return StartCoroutine(MoveVertical(-stepDownY, 1));
        }
    }

    private IEnumerator MoveHorizontal(float stepSize, int steps)
    {
        for (int i = 0; i < steps; i++)
        {
            Vector3 startPos = transform.position;
            Vector3 targetPos = startPos + new Vector3(stepSize, 0f, 0f);

            float t = 0f;
            while (t < 1f)
            {
                t += Time.deltaTime / stepDuration;
                transform.position = Vector3.Lerp(startPos, targetPos, t);
                yield return null;
            }
        }
    }

    private IEnumerator MoveVertical(float stepSize, int steps)
    {
        for (int i = 0; i < steps; i++)
        {
            Vector3 startPos = transform.position;
            Vector3 targetPos = startPos + new Vector3(0f, stepSize, 0f);

            float t = 0f;
            while (t < 1f)
            {
                t += Time.deltaTime / stepDuration;
                transform.position = Vector3.Lerp(startPos, targetPos, t);
                yield return null;
            }
        }
    }
}
